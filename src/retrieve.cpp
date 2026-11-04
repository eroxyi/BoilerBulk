#include "retrieve.hpp"
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

int main() { retrieveData(); }
