#ifndef CURL_GLOBAL_CONTEXT_H_
#define CURL_GLOBAL_CONTEXT_H_

#include <iostream>
#include <memory>

#include "curl/curl.h"
#include "curl_easy_handle.h"

namespace curl {

class GlobalContext {
 public:
  GlobalContext() : failed_(false) {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
      std::cout << "curl_global_init failed" << std::endl;
      failed_ = true;
      return;
    }
  }

  ~GlobalContext() { curl_global_cleanup(); }

  explicit operator bool() const { return !failed_; }
  bool failed() const { return failed_; }

 private:
  bool failed_;
};

}  // namespace curl

#endif  // CURL_GLOBAL_CONTEXT_H_
