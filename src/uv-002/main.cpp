#include <iostream>
#include "uv/uv.h"

int64_t count = 10;

void timeoutCallback(uv_timer_t *timer) {
  std::cout << "count: " << count-- << std::endl;
  if (count < 0) {
    uv_timer_stop(timer);
  }
}

int main(int argc, char *argv[]) {
  uv_loop_t *loop = uv_default_loop();
  uv_timer_t timer;
  uv_timer_init(loop, &timer);
  uv_timer_start(&timer, timeoutCallback, 0, 1000);
  std::cout << "loop: start" << std::endl;
  uv_run(loop, UV_RUN_DEFAULT);
  std::cout << "loop: end" << std::endl;
  uv_loop_close(loop);
  return 0;
}
