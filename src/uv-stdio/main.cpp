#include <cstdlib>
#include "uv.h"

constexpr int kStdin = 0;

int main(int argc, char* argv[]) {
  auto loop = uv_default_loop();
  uv_tty_t tty;
  uv_tty_init(loop, &tty, kStdin, 1);
  uv_read_start(reinterpret_cast<uv_stream_t*>(&tty),
                [](uv_handle_t*, size_t suggested_size, uv_buf_t* buf) {
                  buf->base =
                      reinterpret_cast<char*>(std::malloc(suggested_size));
                  buf->len = suggested_size;
                },
                [](uv_stream_t*, ssize_t nread, const uv_buf_t* buf) {
                  for (size_t i = 0; i < nread; ++i) {
                    putchar(buf->base[i]);
                  }
                  if (buf->base != nullptr) {
                    std::free(buf->base);
                  }
                });
  uv_run(loop, UV_RUN_DEFAULT);
  return 0;
}
