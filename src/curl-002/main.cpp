#include <iostream>
#include <memory>

#include "curl_easy_handle.h"
#include "curl_global_context.h"
#include "curl_multi_handle.h"

class Application {
 public:
  Application() : failed_(false) {}

  ~Application() {}

  bool Run() {
    curl::MultiHandle multi_handle;
    if (!multi_handle) {
      std::cout << "curl::MultiHandle failed" << std::endl;
      return false;
    }

    curl::EasyHandle e1("https://www.google.com/"), e2("https://www.bing.com");
    if (!e1 || !e2) {
      std::cout << "curl::EasyHandle failed" << std::endl;
      return false;
    }

    multi_handle.Add(std::move(e1));
    multi_handle.Add(std::move(e2));

    multi_handle.Perform();
    while (multi_handle.Perform() > 0) {
      multi_handle.Wait();
    }

    return true;
  }

  explicit operator bool() const { return !failed_; }
  bool failed() const { return failed_; }

 private:
  bool failed_;
};

int main(int argc, char **argv) {
  curl::GlobalContext ctx;
  if (!ctx) {
    return 1;
  }
  Application app;
  if (!app) {
    return 1;
  }
  return app.Run() ? 0 : 1;
}
