#pragma once
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

struct PlanItem {
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
  std::map<std::string, Servings> meals;
};

std::optional<DayPlan> planBestCourt(const CourtMenus &courts,
                                     const std::vector<std::string> &meals,
                                     double goal);

void printPlan(const DayPlan &plan, const std::vector<std::string> &meals,
               double goal);
