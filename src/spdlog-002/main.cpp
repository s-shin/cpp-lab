#include <mutex>
#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"

// Ref.
// https://github.com/gabime/spdlog/blob/5585299b038e0e196bfe43719f81cec3f241dbd3/tests/file_log.cpp#L119-L150

struct CustomDailyFileNameCalculator {
  static spdlog::filename_t calc_filename(const spdlog::filename_t& basename) {
    std::tm tm = spdlog::details::os::localtime();
    fmt::MemoryWriter w;
    w.write("{}.{:04d}{:02d}{:02d}", basename, tm.tm_year + 1900, tm.tm_mon + 1,
            tm.tm_mday);
    return w.str();
  }
};

int main(int argc, char* argv[]) {
  spdlog::set_async_mode(1024);
  using sink_type =
      spdlog::sinks::daily_file_sink<std::mutex, CustomDailyFileNameCalculator>;
  auto logger =
      spdlog::create<sink_type>("default", "tmp/log/spdlog-002.log", 0, 0);
  logger->info("async daily logger");
  return 0;
}
