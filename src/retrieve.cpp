#include "retrieve.hpp"
#include <ctime>
#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  size_t totalSize = size * nmemb;
  static_cast<std::string *>(userp)->append(static_cast<char *>(contents),
                                            totalSize);
  return totalSize;
}

static std::string httpGet(const std::string &url) {
  CURL *curl = curl_easy_init();
  std::string readBuffer;

  if (!curl) {
    std::cerr << "Failed to initialize curl handle\n";
    return "";
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    std::cerr << "curl error for " << url << ": " << curl_easy_strerror(res)
              << "\n";
    return "";
  }

  return readBuffer;
}

nlohmann::json retrieveData() {
  std::string response =
      httpGet("https://api.hfs.purdue.edu/menus/v2/locations/");
  if (response.empty())
    return nlohmann::json::object();
  return nlohmann::json::parse(response, nullptr, false);
}

nlohmann::json retrieveData(const std::string &diningCourt) {
  std::time_t t = std::time(nullptr);
  std::tm *local_time = std::localtime(&t);

  char dateBuffer[11];
  std::strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", local_time);

  std::string url = "https://api.hfs.purdue.edu/menus/v2/locations/" +
                    diningCourt + "/" + dateBuffer;

  std::string response = httpGet(url);
  if (response.empty())
    return nlohmann::json::object();
  return nlohmann::json::parse(response, nullptr, false);
}

nlohmann::json retrieveNutrition(const std::string &itemID) {
  std::string url = "https://api.hfs.purdue.edu/menus/v2/items/" + itemID;
  std::string response = httpGet(url);
  if (response.empty())
    return nlohmann::json::object();
  return nlohmann::json::parse(response, nullptr, false);
}
