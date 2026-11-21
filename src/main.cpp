#include <iostream>
#include <string>

#include "optimize.hpp"
#include "top_items.hpp"

namespace {

constexpr double kDefaultGoal = 180.0;

bool parseGoal(int argc, char **argv, double &goal) {
  goal = kDefaultGoal;
  if (argc < 2)
    return true;

  try {
    size_t pos = 0;
    goal = std::stod(argv[1], &pos);
    if (pos != std::string(argv[1]).size() || goal <= 0.0)
      throw std::invalid_argument("bad goal");
  } catch (const std::exception &) {
    std::cerr << "Usage: " << argv[0] << " [protein_goal_grams > 0]\n";
    return false;
  }
  return true;
}

} // namespace

int main(int argc, char **argv) {
  double goal;
  if (!parseGoal(argc, argv, goal))
    return 1;

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
