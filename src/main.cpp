#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include "optimize.hpp"
#include "top_items.hpp"

namespace {

namespace fs = std::filesystem;

constexpr const char *kAppName = "dining-planner";
constexpr const char *kConfigFile = "profile.json";

fs::path configPath() {
  const char *xdg = std::getenv("XDG_CONFIG_HOME");
  if (xdg && *xdg)
    return fs::path(xdg) / kAppName / kConfigFile;

  const char *home = std::getenv("HOME");
  if (home && *home)
    return fs::path(home) / ".config" / kAppName / kConfigFile;

  return fs::path(kConfigFile);
}

bool parseGoal(const std::string &g, Goal &out) {
  if (g == "cut")
    out = Goal::Cut;
  else if (g == "maintain")
    out = Goal::Maintain;
  else if (g == "bulk")
    out = Goal::Bulk;
  else
    return false;
  return true;
}

bool loadProfile(const fs::path &path, Profile &p) {
  std::ifstream in(path);
  if (!in) {
    std::cerr << "No config found at " << path.string()
              << "\nCreate it, e.g.:\n"
                 "{\n  \"weight_kg\": 80,\n  \"goal\": \"cut\"\n}\n";
    return false;
  }

  nlohmann::json j = nlohmann::json::parse(in, nullptr, false);
  if (j.is_discarded() || !j.is_object()) {
    std::cerr << path.string() << ": not a valid JSON object\n";
    return false;
  }

  for (const auto &[key, v] : j.items()) {
    bool ok = true;
    if (key == "weight_kg") {
      ok = v.is_number() && v.get<double>() > 0.0;
      if (ok)
        p.weightKg = v.get<double>();
    } else if (key == "goal") {
      ok = v.is_string() && parseGoal(v.get<std::string>(), p.goal);
    } else if (key == "calorie_target") {
      ok = v.is_number() && v.get<double>() > 0.0;
      if (ok)
        p.calorieTarget = static_cast<int>(std::lround(v.get<double>()));
    } else if (key == "protein_target") {
      ok = v.is_number() && v.get<double>() > 0.0;
      if (ok)
        p.proteinTarget = v.get<double>();
    } else {
      std::cerr << path.string() << ": ignoring unknown key \"" << key
                << "\"\n";
      continue;
    }
    if (!ok) {
      std::cerr << path.string() << ": invalid value for \"" << key << "\"\n";
      return false;
    }
  }

  if (!p.resolve()) {
    std::cerr << path.string()
              << ": \"weight_kg\" is required unless both \"calorie_target\" "
                 "and \"protein_target\" are set\n";
    return false;
  }
  return true;
}

} // namespace

int main(int argc, char **argv) {
  fs::path path = configPath();
  if (argc > 1) {
    std::cerr << "Usage: " << argv[0] << "\nThis program takes no arguments; "
              << "edit " << path.string() << " instead.\n";
    return 1;
  }

  Profile profile;
  if (!loadProfile(path, profile))
    return 1;

  NutritionCache cache;
  MealMenus menus;
  fetchAll(cache, menus);

  CourtMenus byCourt;
  for (const auto &[meal, items] : menus)
    for (const auto &[id, courts] : items)
      for (const std::string &court : courts) {
        const Nutrition &n = cache.at(id);
        byCourt[court][meal].push_back({id, n.name, n.protein, n.calories});
      }

  if (auto plan = planBestCourt(byCourt, kMeals, profile))
    printPlan(*plan, kMeals, profile);
  else
    std::cout << "No items found.\n";

  std::cout << "\n----- Top protein dense items -----\n";
  for (const std::string &meal : kMeals)
    printSection(meal, menus, cache);
  return 0;
}
