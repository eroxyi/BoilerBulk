#include "top_items.hpp"

#include <algorithm>
#include <iostream>
#include <retrieve.hpp>

double scoreFood(double protein, int calories, double healthyFats) {
  return ((protein * 4) / (calories));
}

std::string normalizeMeal(const std::string &name) {
  if (name == "Late Lunch")
    return "Lunch";
  return name;
}

const Nutrition &getNutrition(const std::string &itemId,
                              const std::string &itemName,
                              NutritionCache &cache) {
  auto it = cache.find(itemId);
  if (it != cache.end())
    return it->second;

  Nutrition n;
  n.name = itemName;

  try {
    nlohmann::json details = retrieveNutrition(itemId);

    if (!details.is_discarded() && details.contains("Nutrition") &&
        details["Nutrition"].is_array()) {
      for (const auto &nutrient : details["Nutrition"]) {
        std::string nName = nutrient.value("Name", "");
        if (nName == "Protein") {
          n.protein = nutrient.value("Value", 0.0);
        } else if (nName == "Calories") {
          n.calories = static_cast<int>(nutrient.value("Value", 0.0));
        } else if (nName == "Total Fat") {
          n.fat = nutrient.value("Value", 0.0);
        }
      }

      if (n.calories - n.fat * 9.0 > 0.0 && n.protein >= kMinProtein) {
        n.score = scoreFood(n.protein, n.calories, n.fat);
        n.valid = true;
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Error parsing item " << itemName << ": " << e.what() << "\n";
  }

  return cache.emplace(itemId, n).first->second;
}

void collectItems(const nlohmann::json &j, const std::string &court,
                  NutritionCache &cache, MealMenus &menus) {
  if (!j.contains("Meals") || !j["Meals"].is_array())
    return;

  for (const auto &meal : j["Meals"]) {
    std::string mealName = normalizeMeal(meal.value("Name", ""));
    if (std::find(kMeals.begin(), kMeals.end(), mealName) == kMeals.end())
      continue;
    if (!meal.contains("Stations") || !meal["Stations"].is_array())
      continue;

    for (const auto &station : meal["Stations"]) {
      if (!station.contains("Items") || !station["Items"].is_array())
        continue;

      for (const auto &item : station["Items"]) {
        if (!item.contains("ID") || !item.contains("Name"))
          continue;
        if (!item.value("NutritionReady", false))
          continue;

        std::string itemId = item["ID"].get<std::string>();
        std::string itemName = item["Name"].get<std::string>();

        const Nutrition &n = getNutrition(itemId, itemName, cache);
        if (!n.valid)
          continue;

        auto &courts = menus[mealName][itemId];
        if (std::find(courts.begin(), courts.end(), court) == courts.end())
          courts.push_back(court);
      }
    }
  }
}

std::vector<std::string> getDiningCourts() {
  std::vector<std::string> courts;
  nlohmann::json locations = retrieveData();

  if (locations.is_discarded() || !locations.contains("Location") ||
      !locations["Location"].is_array())
    return courts;

  for (const auto &loc : locations["Location"]) {
    if (loc.value("Type", "") == "Dining Courts") {
      courts.push_back(loc.value("Name", ""));
    }
  }
  return courts;
}

void fetchAll(NutritionCache &cache, MealMenus &menus) {
  for (const std::string &court : getDiningCourts()) {
    std::cerr << "Fetching " << court << "...\n";
    nlohmann::json menu = retrieveData(court);
    if (menu.is_discarded())
      continue;
    collectItems(menu, court, cache, menus);
  }
}

void printSection(const std::string &meal, const MealMenus &menus,
                  const NutritionCache &cache) {
  std::cout << "\n=== " << meal << " ===\n";

  auto mit = menus.find(meal);
  if (mit == menus.end() || mit->second.empty()) {
    std::cout << "(no data)\n";
    return;
  }

  struct Row {
    const Nutrition *n;
    const std::vector<std::string> *courts;
  };

  std::vector<Row> rows;
  for (const auto &[id, courts] : mit->second) {
    rows.push_back({&cache.at(id), &courts});
  }

  size_t n = std::min(kTopN, rows.size());
  std::partial_sort(
      rows.begin(), rows.begin() + n, rows.end(),
      [](const Row &a, const Row &b) { return a.n->score > b.n->score; });

  for (size_t i = 0; i < n; ++i) {
    const Nutrition &f = *rows[i].n;
    std::cout << i + 1 << ". " << f.name << " [";
    for (size_t c = 0; c < rows[i].courts->size(); ++c)
      std::cout << (c ? ", " : "") << (*rows[i].courts)[c];
    std::cout << "]: " << f.score << " (" << f.protein << "g protein, "
              << f.calories << " cal)\n";
  }
}
