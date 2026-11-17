#pragma once
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

constexpr size_t kTopN = 15;
constexpr double kMinProtein = 10;

const std::vector<std::string> kMeals = {"Breakfast", "Lunch", "Dinner"};

struct Nutrition {
  std::string name;
  double protein = 0.0;
  double fat = 0.0;
  int calories = 0;
  double score = 0.0;
  bool valid = false;
};

using NutritionCache = std::unordered_map<std::string, Nutrition>;
using MealMenus =
    std::map<std::string,
             std::unordered_map<std::string, std::vector<std::string>>>;

double scoreFood(double protein, int calories, double healthyFats);
std::string normalizeMeal(const std::string &name);

const Nutrition &getNutrition(const std::string &itemId,
                              const std::string &itemName,
                              NutritionCache &cache);

void collectItems(const nlohmann::json &j, const std::string &court,
                  NutritionCache &cache, MealMenus &menus);

std::vector<std::string> getDiningCourts();

void fetchAll(NutritionCache &cache, MealMenus &menus);

void printSection(const std::string &meal, const MealMenus &menus,
                  const NutritionCache &cache);
