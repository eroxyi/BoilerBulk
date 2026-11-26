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
constexpr const char *kExternalFoodFile = "external_food.json";

fs::path configPath() {
  const char *xdg = std::getenv("XDG_CONFIG_HOME");
  if (xdg && *xdg) {
    fs::path p = fs::path(xdg) / kAppName / kConfigFile;
    if (fs::exists(p))
      return p;
  }

  const char *home = std::getenv("HOME");
  if (home && *home) {
    fs::path p = fs::path(home) / ".config" / kAppName / kConfigFile;
    if (fs::exists(p))
      return p;
  }

  if (fs::exists(fs::path("data") / kConfigFile))
    return fs::path("data") / kConfigFile;

  return fs::path(kConfigFile);
}

fs::path externalFoodPath() {
  const char *xdg = std::getenv("XDG_CONFIG_HOME");
  if (xdg && *xdg) {
    fs::path p = fs::path(xdg) / kAppName / kExternalFoodFile;
    if (fs::exists(p))
      return p;
  }

  const char *home = std::getenv("HOME");
  if (home && *home) {
    fs::path p = fs::path(home) / ".config" / kAppName / kExternalFoodFile;
    if (fs::exists(p))
      return p;
  }

  if (fs::exists(fs::path("data") / kExternalFoodFile))
    return fs::path("data") / kExternalFoodFile;

  return fs::path(kExternalFoodFile);
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

bool parseFoodItem(const nlohmann::json &item, ExternalFood &food) {
  if (!item.is_object())
    return false;

  if (item.contains("name") && item["name"].is_string())
    food.name = item["name"].get<std::string>();
  else if (item.contains("item") && item["item"].is_string())
    food.name = item["item"].get<std::string>();
  else
    return false;

  if (item.contains("calories") && item["calories"].is_number())
    food.calories = static_cast<int>(std::lround(item["calories"].get<double>()));
  else if (item.contains("kcal") && item["kcal"].is_number())
    food.calories = static_cast<int>(std::lround(item["kcal"].get<double>()));
  else
    food.calories = 0;

  if (item.contains("protein_g") && item["protein_g"].is_number())
    food.protein = item["protein_g"].get<double>();
  else if (item.contains("protein") && item["protein"].is_number())
    food.protein = item["protein"].get<double>();
  else
    food.protein = 0.0;

  if (item.contains("quantity") && item["quantity"].is_number())
    food.quantity = static_cast<int>(std::lround(item["quantity"].get<double>()));
  else if (item.contains("servings") && item["servings"].is_number())
    food.quantity = static_cast<int>(std::lround(item["servings"].get<double>()));
  else
    food.quantity = 1;

  if (food.quantity <= 0)
    food.quantity = 1;

  return true;
}

bool loadExternalFoods(const fs::path &path, Profile &p) {
  if (!fs::exists(path))
    return true;

  std::ifstream in(path);
  if (!in)
    return false;

  nlohmann::json j = nlohmann::json::parse(in, nullptr, false);
  if (j.is_discarded()) {
    std::cerr << path.string() << ": not valid JSON\n";
    return false;
  }

  if (j.is_array()) {
    for (const auto &elem : j) {
      ExternalFood food;
      if (parseFoodItem(elem, food))
        p.addExternalFood(food);
    }
  } else if (j.is_object()) {
    for (const auto &[key, v] : j.items()) {
      if ((key == "foods" || key == "external_foods") && v.is_array()) {
        for (const auto &elem : v) {
          ExternalFood food;
          if (parseFoodItem(elem, food))
            p.addExternalFood(food);
        }
      }
    }
  }
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
    } else if (key == "foods" || key == "external_foods") {
      if (v.is_array()) {
        for (const auto &elem : v) {
          ExternalFood food;
          if (parseFoodItem(elem, food))
            p.addExternalFood(food);
        }
      }
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

}

int main(int argc, char **argv) {
  fs::path path = configPath();
  fs::path extPath = externalFoodPath();

  if (argc > 1) {
    path = argv[1];
  }
  if (argc > 2) {
    extPath = argv[2];
  }
  if (argc > 3) {
    std::cerr << "Usage: " << argv[0] << " [profile.json] [external_food.json]\n";
    return 1;
  }

  Profile profile;
  if (!loadProfile(path, profile))
    return 1;

  std::error_code ec;
  if (fs::exists(extPath) && !fs::equivalent(path, extPath, ec)) {
    if (!loadExternalFoods(extPath, profile))
      return 1;
  }

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

  if (auto plan = planBestCourt(byCourt, kMeals, profile)) {
    printPlan(*plan, kMeals, profile);
    fs::create_directories("output");
    savePlan("output/daily_plan.txt", *plan, kMeals, profile);
  } else {
    std::cout << "No items found.\n";
  }

  std::cout << "\n----- Top protein dense items -----\n";
  for (const std::string &meal : kMeals)
    printSection(meal, menus, cache);
  return 0;
}
