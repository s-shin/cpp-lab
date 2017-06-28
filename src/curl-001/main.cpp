#include <iostream>
#include <memory>

#include "curl/curl.h"

size_t WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  std::cout.write(ptr, size * nmemb);
  return size * nmemb;
}

int main(int argc, char** argv) {
  if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
    std::cout << "curl_global_init failed" << std::endl;
  }

  std::shared_ptr<CURL> curl(curl_easy_init(), curl_easy_cleanup);
  if (!curl) {
    std::cout << "curl_easy_init failed" << std::endl;
    return 1;
  }

  curl_easy_setopt(curl.get(), CURLOPT_URL, "https://www.google.com/");
  curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);

  auto res = curl_easy_perform(curl.get());
  if (res != CURLE_OK) {
    std::cout << "curl_easy_perform failed with " << curl_easy_strerror(res)
              << std::endl;
    return -1;
  }

  return 0;
}
