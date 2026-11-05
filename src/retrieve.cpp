// #include "retrieve.hpp"
#include <ctime>
#include <curl/curl.h>
#include <iostream>

void retrieveData() {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl) {
    FILE *fp = fopen("../data/downloaded.json", "wb");

    if (!fp) {
      std::cout << "Something went wrong when trying to open file\n";
      curl_easy_cleanup(curl);
      return;
    }

    curl_easy_setopt(curl, CURLOPT_URL,
                     "http://api.hfs.purdue.edu/menus/v2/locations/");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      std::cout << "Something went wrong with curling the data\n";
      curl_easy_cleanup(curl);
      return;
    } else {
      std::cout << "File was downloaded successfully\n";
    }

    fclose(fp);
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  return;
}

void retrieveData(std::string diningCourt) {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl) {
    // get the time
    std::time_t time = std::time(nullptr);
    std::tm *local_time = std::localtime(&time);

    char buffer[11];
    std::strftime(buffer, sizeof(buffer), "%m-%d-%Y", local_time);

    std::string fileName = "../data/" + diningCourt + buffer + ".json";
    FILE *fp = fopen(fileName.c_str(), "wb");

    if (!fp) {
      std::cout << "Something went wrong when trying to open file\n";
      curl_easy_cleanup(curl);
      return;
    }

    std::string curlUrl =
        std::string("http://api.hfs.purdue.edu/menus/v2/locations/") +
        diningCourt + "/" + buffer;

    std::cout << "LOGGING: " << curlUrl << std::endl;

    curl_easy_setopt(curl, CURLOPT_URL, curlUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      std::cout << "Something went wrong with curling the data\n";
      curl_easy_cleanup(curl);
      return;
    } else {
      std::cout << "File was downloaded successfully\n";
    }

    fclose(fp);
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  return;
}

int main() {
  retrieveData();
  retrieveData("Earhart");
}
