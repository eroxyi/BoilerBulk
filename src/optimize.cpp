#include "optimize.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

namespace {

constexpr int kMaxServings = 4;
constexpr double kEps = 1e-9;

constexpr double kProteinWeight = 1.0;
constexpr double kCalorieWeight = 0.1;
constexpr double kMaintainBand = 0.05;

struct Chunk {
  const PlanItem *item;
  int servings;
  double protein;
  int calories;
};

struct MealTable {
  std::vector<Chunk> chunks;
  std::vector<std::vector<double>> dp;
  const std::vector<double> &frontier() const { return dp.back(); }
};

MealTable solveMeal(const std::vector<PlanItem> &items, int maxCal) {
  MealTable t;
  for (const PlanItem &item : items) {
    int remaining = kMaxServings;
    for (int k = 1; remaining > 0; k *= 2) {
      int take = std::min(k, remaining);
      remaining -= take;
      if (item.calories * take > maxCal)
        continue;
      t.chunks.push_back(
          {&item, take, item.protein * take, item.calories * take});
    }
  }

  t.dp.assign(t.chunks.size() + 1, std::vector<double>(maxCal + 1, 0.0));
  for (size_t j = 1; j <= t.chunks.size(); ++j) {
    const Chunk &ch = t.chunks[j - 1];
    for (int c = 0; c <= maxCal; ++c) {
      t.dp[j][c] = t.dp[j - 1][c];
      if (c >= ch.calories)
        t.dp[j][c] =
            std::max(t.dp[j][c], t.dp[j - 1][c - ch.calories] + ch.protein);
    }
  }
  return t;
}

Servings backtrackMeal(const MealTable &t, int budget) {
  std::map<std::string, std::pair<PlanItem, int>> acc;
  int c = budget;
  for (size_t j = t.chunks.size(); j >= 1; --j) {
    if (t.dp[j][c] != t.dp[j - 1][c]) {
      const Chunk &ch = t.chunks[j - 1];
      auto &entry = acc[ch.item->id];
      entry.first = *ch.item;
      entry.second += ch.servings;
      c -= ch.calories;
    }
  }
  Servings out;
  for (auto &[id, e] : acc)
    out.push_back(e);
  return out;
}

std::vector<double> convolve(const std::vector<double> &A,
                             const std::vector<double> &B,
                             std::vector<int> &split, int maxCal) {
  std::vector<double> R(maxCal + 1, 0.0);
  split.assign(maxCal + 1, 0);
  for (int c = 0; c <= maxCal; ++c) {
    double best = -1.0;
    int bestC1 = 0;
    for (int c1 = 0; c1 <= c; ++c1) {
      double v = A[c1] + B[c - c1];
      if (v > best) {
        best = v;
        bestC1 = c1;
      }
    }
    R[c] = best;
    split[c] = bestC1;
  }
  return R;
}

double calorieDeviation(const Profile &p, int c) {
  const double T = p.remainingCalorieTarget;
  switch (p.goal) {
  case Goal::Cut:
    return std::max(0.0, c - T);
  case Goal::Bulk:
    return std::max(0.0, T - c);
  case Goal::Maintain:
    return std::max(0.0, std::abs(c - T) - kMaintainBand * T);
  }
  return 0.0;
}

double planCost(const Profile &p, double protein, int calories) {
  return kProteinWeight * std::max(0.0, p.remainingProteinTarget - protein) +
         kCalorieWeight * calorieDeviation(p, calories);
}

const char *goalName(Goal g) {
  switch (g) {
  case Goal::Cut:
    return "cut";
  case Goal::Maintain:
    return "maintain";
  case Goal::Bulk:
    return "bulk";
  }
  return "";
}

DayPlan planCourt(const std::string &court, const CourtMenu &menu,
                  const std::vector<std::string> &meals,
                  const Profile &profile) {
  static const std::vector<PlanItem> kEmpty;
  const int maxCal = profile.maxCalories();

  std::vector<MealTable> tables;
  for (const std::string &meal : meals) {
    auto it = menu.find(meal);
    tables.push_back(solveMeal(it == menu.end() ? kEmpty : it->second, maxCal));
  }

  std::vector<double> F = tables[0].frontier();
  std::vector<std::vector<int>> splits(tables.size());
  for (size_t m = 1; m < tables.size(); ++m)
    F = convolve(F, tables[m].frontier(), splits[m], maxCal);

  int target = 0;
  double bestCost = planCost(profile, F[0], 0);
  for (int c = 1; c <= maxCal; ++c) {
    double cost = planCost(profile, F[c], c);
    if (cost < bestCost - kEps) {
      bestCost = cost;
      target = c;
    }
  }
  bool reached = F[target] >= profile.remainingProteinTarget - kEps;

  std::vector<int> budget(tables.size(), 0);
  int c = target;
  for (size_t m = tables.size() - 1; m >= 1; --m) {
    int upTo = splits[m][c];
    budget[m] = c - upTo;
    c = upTo;
  }
  budget[0] = c;

  DayPlan plan;
  plan.court = court;
  plan.reached = reached;
  for (size_t m = 0; m < tables.size(); ++m) {
    plan.meals[meals[m]] = backtrackMeal(tables[m], budget[m]);
    for (const auto &[item, s] : plan.meals[meals[m]]) {
      plan.protein += item.protein * s;
      plan.calories += item.calories * s;
    }
  }
  plan.cost = planCost(profile, plan.protein, plan.calories);
  return plan;
}

bool better(const DayPlan &a, const DayPlan &b) {
  if (std::abs(a.cost - b.cost) > kEps)
    return a.cost < b.cost;
  return a.calories < b.calories;
}

}

std::optional<DayPlan> planBestCourt(const CourtMenus &courts,
                                     const std::vector<std::string> &meals,
                                     const Profile &profile) {
  if (meals.empty())
    return std::nullopt;

  std::optional<DayPlan> best;
  for (const auto &[court, menu] : courts) {
    DayPlan p = planCourt(court, menu, meals, profile);
    if (!best || better(p, *best))
      best = std::move(p);
  }
  return best;
}

void printPlan(std::ostream &os, const DayPlan &plan,
               const std::vector<std::string> &meals,
               const Profile &profile) {
  os << "\n----- Best court: " << plan.court << " ("
     << goalName(profile.goal) << ") -----\n";

  if (!profile.externalFoods.empty()) {
    os << "\n=== Logged External Foods ===\n";
    for (const auto &item : profile.externalFoods) {
      os << item.quantity << "x " << item.name << " ("
         << item.protein * item.quantity << "g protein, "
         << item.calories * item.quantity << " cal)\n";
    }
    os << "External Total: " << profile.externalProtein << "g protein, "
       << profile.externalCalories << " cal\n";
    os << "Remaining Dining Targets: " << profile.remainingProteinTarget
       << "g protein, " << profile.remainingCalorieTarget << " cal\n";
  }

  if (!plan.reached)
    os << "(could not reach " << profile.remainingProteinTarget
       << "g protein; showing best trade-off)\n";

  for (const std::string &meal : meals) {
    os << "\n=== " << meal << " ===\n";
    const Servings &s = plan.meals.at(meal);
    if (s.empty())
      os << "(nothing)\n";
    for (const auto &[item, n] : s)
      os << n << "x " << item.name << " (" << item.protein * n
         << "g protein, " << item.calories * n << " cal)\n";
  }

  os << "\nDining Total: " << plan.protein << "g protein, " << plan.calories
     << " cal\n";
  if (!profile.externalFoods.empty()) {
    os << "Daily Total (including external): "
       << (plan.protein + profile.externalProtein) << "g protein, "
       << (plan.calories + profile.externalCalories) << " cal\n";
  }
  os << "Daily Targets: " << profile.proteinTarget << "g protein, "
     << profile.calorieTarget << " cal\n";
}

void printPlan(const DayPlan &plan, const std::vector<std::string> &meals,
               const Profile &profile) {
  printPlan(std::cout, plan, meals, profile);
}

void savePlan(const std::string &path, const DayPlan &plan,
              const std::vector<std::string> &meals,
              const Profile &profile) {
  std::ofstream out(path);
  if (out)
    printPlan(out, plan, meals, profile);
}
