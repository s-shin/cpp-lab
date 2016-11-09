#include <iostream>
#include "uv/uv.h"

// https://nikhilm.github.io/uvbook/basics.html

int64_t counter = 0;

void wait(uv_idle_t *handle) {
  counter++;
  if (counter > 10e6) {
    uv_idle_stop(handle);
  }
}

int main(int argc, char *argv[]) {
  uv_loop_t *loop = uv_default_loop();
  uv_idle_t idler;
  uv_idle_init(loop, &idler);
  uv_idle_start(&idler, wait);
  std::cout << "Idling..." << std::endl;
  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);
  return 0;
}
