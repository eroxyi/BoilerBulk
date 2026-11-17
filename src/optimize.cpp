#include "optimize.hpp"

#include <algorithm>
#include <iostream>

namespace {

constexpr int kMaxCalories = 4000;
constexpr int kMaxServings = 4;
constexpr double kEps = 1e-9;

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

MealTable solveMeal(const std::vector<PlanItem> &items) {
  MealTable t;
  for (const PlanItem &item : items) {
    int remaining = kMaxServings;
    for (int k = 1; remaining > 0; k *= 2) {
      int take = std::min(k, remaining);
      remaining -= take;
      if (item.calories * take > kMaxCalories)
        continue;
      t.chunks.push_back(
          {&item, take, item.protein * take, item.calories * take});
    }
  }

  t.dp.assign(t.chunks.size() + 1, std::vector<double>(kMaxCalories + 1, 0.0));
  for (size_t j = 1; j <= t.chunks.size(); ++j) {
    const Chunk &ch = t.chunks[j - 1];
    for (int c = 0; c <= kMaxCalories; ++c) {
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
      auto &entry = acc[ch.item->name];
      entry.first = *ch.item;
      entry.second += ch.servings;
      c -= ch.calories;
    }
  }
  Servings out;
  for (auto &[name, e] : acc)
    out.push_back(e);
  return out;
}

std::vector<double> convolve(const std::vector<double> &A,
                             const std::vector<double> &B,
                             std::vector<int> &split) {
  std::vector<double> R(kMaxCalories + 1, 0.0);
  split.assign(kMaxCalories + 1, 0);
  for (int c = 0; c <= kMaxCalories; ++c) {
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

DayPlan planCourt(const std::string &court, const CourtMenu &menu,
                  const std::vector<std::string> &meals, double goal) {
  static const std::vector<PlanItem> kEmpty;

  std::vector<MealTable> tables;
  for (const std::string &meal : meals) {
    auto it = menu.find(meal);
    tables.push_back(solveMeal(it == menu.end() ? kEmpty : it->second));
  }

  std::vector<double> F = tables[0].frontier();
  std::vector<std::vector<int>> splits(tables.size());
  for (size_t m = 1; m < tables.size(); ++m)
    F = convolve(F, tables[m].frontier(), splits[m]);

  int target = kMaxCalories;
  bool reached = false;
  for (int c = 0; c <= kMaxCalories; ++c) {
    if (F[c] >= goal - kEps) {
      target = c;
      reached = true;
      break;
    }
  }

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
  return plan;
}

bool better(const DayPlan &a, const DayPlan &b) {
  if (a.reached != b.reached)
    return a.reached;
  if (a.reached)
    return a.calories < b.calories;
  return a.protein > b.protein;
}

} // namespace

std::optional<DayPlan> planBestCourt(const CourtMenus &courts,
                                     const std::vector<std::string> &meals,
                                     double goal) {
  if (meals.empty())
    return std::nullopt;

  std::optional<DayPlan> best;
  for (const auto &[court, menu] : courts) {
    DayPlan p = planCourt(court, menu, meals, goal);
    if (!best || better(p, *best))
      best = std::move(p);
  }
  return best;
}

void printPlan(const DayPlan &plan, const std::vector<std::string> &meals,
               double goal) {
  std::cout << "\n----- Best court: " << plan.court << " -----\n";
  if (!plan.reached)
    std::cout << "(could not reach " << goal << "g within " << kMaxCalories
              << " cal; showing max protein)\n";

  for (const std::string &meal : meals) {
    std::cout << "\n=== " << meal << " ===\n";
    const Servings &s = plan.meals.at(meal);
    if (s.empty())
      std::cout << "(nothing)\n";
    for (const auto &[item, n] : s)
      std::cout << n << "x " << item.name << " (" << item.protein * n
                << "g protein, " << item.calories * n << " cal)\n";
  }
  std::cout << "\nTotal: " << plan.protein << "g protein, " << plan.calories
            << " cal (goal " << goal << "g)\n";
}
