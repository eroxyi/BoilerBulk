#pragma once

#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <vector>

struct foodItem {
  std::string name;
  double score;
};

struct CompareScore {
  bool operator()(const foodItem &a, const foodItem &b) const {
    return a.score < b.score;
  }
};

using FoodPriorityQueue =
    std::priority_queue<foodItem, std::vector<foodItem>, CompareScore>;

double scoreFood(double protein, int calories, double healthyFats);

void recursive_iterate(const nlohmann::json &j, FoodPriorityQueue &topFive);

void testTopFive(const std::string &diningCourt, FoodPriorityQueue &topFive);

double scoreFood(double protein, int calories, double healthyFats);
