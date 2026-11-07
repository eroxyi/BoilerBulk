#include <nlohmann/json.hpp>
#include <string>

#pragma once

nlohmann::json retrieveData();
nlohmann::json retrieveData(const std::string &diningCourt);
nlohmann::json retrieveNutrition(const std::string &itemID);
