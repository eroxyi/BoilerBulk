#pragma once
#include <algorithm>
#include <cmath>
#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

struct PlanItem {
  std::string id;
  std::string name;
  double protein = 0.0;
  int calories = 0;
};

using CourtMenu = std::map<std::string, std::vector<PlanItem>>;
using CourtMenus = std::map<std::string, CourtMenu>;
using Servings = std::vector<std::pair<PlanItem, int>>;

struct DayPlan {
  std::string court;
  bool reached = false;
  double protein = 0.0;
  int calories = 0;
  double cost = 0.0;
  std::map<std::string, Servings> meals;
};

enum class Goal { Cut, Maintain, Bulk };

struct ExternalFood {
  std::string name;
  int calories = 0;
  double protein = 0.0;
  int quantity = 1;
};

struct Profile {
  static constexpr double kCaloriesPerKg = 50.0;
  static constexpr int kDefaultMaxCalories = 4000;
  static constexpr double kTargetHeadroom = 1.25;

  double weightKg = 0.0;
  Goal goal = Goal::Maintain;
  int calorieTarget = 0;
  double proteinTarget = 0.0;

  int externalCalories = 0;
  double externalProtein = 0.0;
  int remainingCalorieTarget = 0;
  double remainingProteinTarget = 0.0;
  std::vector<ExternalFood> externalFoods;

  static double kcalPerKg(Goal g) {
    switch (g) {
    case Goal::Cut:
      return 27.0;
    case Goal::Maintain:
      return 33.0;
    case Goal::Bulk:
      return 38.0;
    }
    return 33.0;
  }

  static double proteinPerKg(Goal g) {
    switch (g) {
    case Goal::Cut:
      return 2.2;
    case Goal::Maintain:
      return 1.8;
    case Goal::Bulk:
      return 2.0;
    }
    return 1.8;
  }

  void recalculateRemaining() {
    remainingCalorieTarget = std::max(0, calorieTarget - externalCalories);
    remainingProteinTarget = std::max(0.0, proteinTarget - externalProtein);
  }

  void addExternalFood(const ExternalFood &food) {
    externalFoods.push_back(food);
    externalCalories += food.calories * food.quantity;
    externalProtein += food.protein * food.quantity;
    recalculateRemaining();
  }

  bool resolve() {
    if ((calorieTarget <= 0 || proteinTarget <= 0.0) && weightKg <= 0.0)
      return false;
    if (calorieTarget <= 0)
      calorieTarget = static_cast<int>(std::lround(kcalPerKg(goal) * weightKg));
    if (proteinTarget <= 0.0)
      proteinTarget = proteinPerKg(goal) * weightKg;
    recalculateRemaining();
    return true;
  }

  int maxCalories() const {
    int byWeight = weightKg > 0.0
                       ? static_cast<int>(std::ceil(kCaloriesPerKg * weightKg))
                       : kDefaultMaxCalories;
    int byTarget = static_cast<int>(std::ceil(kTargetHeadroom * remainingCalorieTarget));
    return std::max(byWeight, byTarget);
  }
};

std::optional<DayPlan> planBestCourt(const CourtMenus &courts,
                                     const std::vector<std::string> &meals,
                                     const Profile &profile);

void printPlan(std::ostream &os, const DayPlan &plan,
               const std::vector<std::string> &meals,
               const Profile &profile);

void printPlan(const DayPlan &plan, const std::vector<std::string> &meals,
               const Profile &profile);

void savePlan(const std::string &path, const DayPlan &plan,
              const std::vector<std::string> &meals,
              const Profile &profile);
