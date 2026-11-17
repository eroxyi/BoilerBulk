#include <iostream>
#include <string>

#include "optimize.hpp"
#include "top_items.hpp"

int main(int argc, char **argv) {
  double goal = argc > 1 ? std::stod(argv[1]) : 180.0;

  NutritionCache cache;
  MealMenus menus;
  fetchAll(cache, menus);

  CourtMenus byCourt;
  for (const auto &[meal, items] : menus)
    for (const auto &[id, courts] : items)
      for (const std::string &court : courts) {
        const Nutrition &n = cache.at(id);
        byCourt[court][meal].push_back({n.name, n.protein, n.calories});
      }

  if (auto plan = planBestCourt(byCourt, kMeals, goal))
    printPlan(*plan, kMeals, goal);
  else
    std::cout << "No items found.\n";

  std::cout << "\n----- Top protein dense items -----\n";
  for (const std::string &meal : kMeals)
    printSection(meal, menus, cache);
  return 0;
}
