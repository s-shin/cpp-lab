#ifndef CURL_MULTI_HANDLE_H_
#define CURL_MULTI_HANDLE_H_

#include <memory>
#include <vector>

#include "curl/curl.h"
#include "curl_easy_handle.h"

namespace curl {

class MultiHandle {
 public:
  MultiHandle()
      : failed_(false), handle_(curl_multi_init(), curl_multi_cleanup) {
    if (!handle_) {
      failed_ = true;
      return;
    }
  }

  ~MultiHandle() {
    easy_handles_.clear();
    handle_.reset();
  }

  void Add(EasyHandle &&easy) {
    curl_multi_add_handle(handle_.get(), easy.raw_handle());
    easy_handles_.emplace_back(std::move(easy));
  }

  int Perform() {
    int still_running;
    std::cout << "handle: " << handle_.get() << std::endl;
    curl_multi_perform(handle_.get(), &still_running);
    return still_running;
  }

  int Wait() {
    int numfds;
    auto mc = curl_multi_wait(handle_.get(), nullptr, 0, 1000, &numfds);
    if (mc != CURLM_OK) {
      std::cout << "curl_multi_wait failed with " << mc << std::endl;
      return -1;
    }
    return numfds;
  }

  explicit operator bool() const { return !failed_; }
  bool failed() const { return failed_; }

 private:
  bool failed_;
  std::unique_ptr<CURLM, decltype(&curl_multi_cleanup)> handle_;
  std::vector<EasyHandle> easy_handles_;
};

}  // namespace curl

#endif  // CURL_MULTI_HANDLE_H_
