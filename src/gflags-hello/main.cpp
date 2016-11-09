#include <iostream>
#include "gflags/gflags.h"

DEFINE_string(message, "C++11", "");

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::cout << "Hello, " << FLAGS_message << "." << std::endl;
  return 0;
}
