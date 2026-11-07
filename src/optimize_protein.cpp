#include <iostream>
#include <nlohmann/json.hpp>
#include <optimize_protein.hpp>
#include <queue>
#include <retrieve.hpp>

void recursive_iterate(const nlohmann::json &j,
                       std::priority_queue<foodItem, std::vector<foodItem>,
                                           CompareScore> &topFive) {
  if (!j.contains("Meals") || !j["Meals"].is_array())
    return;

  for (const auto &meal : j["Meals"]) {
    if (!meal.contains("Stations") || !meal["Stations"].is_array())
      continue;

    for (const auto &station : meal["Stations"]) {
      if (!station.contains("Items") || !station["Items"].is_array())
        continue;

      for (const auto &item : station["Items"]) {
        if (!item.contains("ID") || !item.contains("Name"))
          continue;

        std::string itemId = item["ID"].get<std::string>();
        std::string itemName = item["Name"].get<std::string>();

        if (item.value("NutritionReady", false)) {
          try {
            nlohmann::json details = retrieveNutrition(itemId);

            if (details.contains("Nutrition") &&
                details["Nutrition"].is_array()) {
              double protein = 0.0;
              int calories = 0;
              double totalFat = 0.0;

              for (const auto &nutrient : details["Nutrition"]) {
                std::string nName = nutrient.value("Name", "");
                if (nName == "Protein") {
                  protein = nutrient.value("Value", 0.0);
                } else if (nName == "Calories") {
                  calories = static_cast<int>(nutrient.value("Value", 0.0));
                } else if (nName == "Total Fat") {
                  totalFat = nutrient.value("Value", 0.0);
                }
              }

              double denominator = calories - (totalFat * 9.0);
              if (denominator > 0.0) {
                double score = scoreFood(protein, calories, totalFat);
                topFive.push(foodItem{itemName, score});
              }
            }
          } catch (const std::exception &e) {
            std::cerr << "Error parsing item " << itemName << ": " << e.what()
                      << "\n";
          }
        }
      }
    }
  }
}

void testTopFive(const std::string &diningCourt, FoodPriorityQueue &topFive) {
  nlohmann::json temp = retrieveData(diningCourt);
  recursive_iterate(temp, topFive);

  for (int i = 0; i < 5 && !topFive.empty(); ++i) {
    foodItem temp = topFive.top();

    std::cout << temp.name << ": " << temp.score << std::endl;
    topFive.pop();
  }
};

double scoreFood(double protein, int calories, double healthyFats) {
  return ((protein * 4) / (calories - (healthyFats * 9)));
}

int main() {
  FoodPriorityQueue topFive;
  testTopFive("Ford", topFive);
  return 0;
}
