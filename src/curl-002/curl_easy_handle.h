#ifndef CURL_EASY_HANDLE_H_
#define CURL_EASY_HANDLE_H_

#include <iostream>
#include <memory>

#include "curl/curl.h"

namespace curl {

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  std::cout.write(ptr, size * nmemb);
  return size * nmemb;
}

class EasyHandle {
 public:
  EasyHandle(const std::string& url)
      : failed_(false), handle_(curl_easy_init(), curl_easy_cleanup) {
    if (!handle_) {
      std::cout << "curl_easy_init failed" << std::endl;
      failed_ = true;
      return;
    }
    curl_easy_setopt(handle_.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle_.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(handle_.get(), CURLOPT_WRITEDATA, this);
  }

  EasyHandle(EasyHandle&& other) noexcept
      : handle_(nullptr, curl_easy_cleanup) {
    *this = std::move(other);
  }

  EasyHandle& operator=(EasyHandle&& other) noexcept {
    if (this != &other) {
      failed_ = other.failed_;
      handle_ = std::move(other.handle_);
    }
    return *this;
  }

  ~EasyHandle() {
    curl_easy_setopt(handle_.get(), CURLOPT_WRITEDATA, nullptr);
    handle_.reset();
  }

  bool Perform() {
    auto res = curl_easy_perform(handle_.get());
    return res == CURLE_OK;
  }

  explicit operator bool() const { return !failed_; }
  bool failed() const { return failed_; }

  CURL* raw_handle() const { return handle_.get(); }

 private:
  bool failed_;
  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> handle_;
};

}  // namespace curl

#endif  // CURL_EASY_HANDLE_H_
