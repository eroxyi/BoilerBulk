#include <string>

#pragma once

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp);
void retrieveData();
void retrieveData(std::string diningCourt);
std::string retrieveNutrition(const std::string itemID);
