#include <iostream>
#include <string>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "gflags/gflags.h"

DEFINE_int32(bufsize, 16, "Read buffer size.");
DEFINE_bool(non_blocking, false, "Enable non-blocking mode.");

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_non_blocking) {
    int ret = fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    if (ret < 0) {
      std::cout << "fctl(2) error: [" << errno << "] " << strerror(errno)
                << std::endl;
      return 1;
    }
  }

  char buf[FLAGS_bufsize];
  ssize_t size = read(STDIN_FILENO, buf, FLAGS_bufsize);
  if (size > 0) {
    std::cout << "Read " << size << " bytes." << std::endl;
    std::string str(buf, size);
    std::cout << "Contents: ---" << std::endl
              << str << std::endl
              << "---" << std::endl;
  } else if (size == 0) {
    std::cout << "EOF" << std::endl;
  } else {
    std::cout << "read(2) error: [" << errno << "] " << strerror(errno)
              << std::endl;
  }

  return 0;
}
