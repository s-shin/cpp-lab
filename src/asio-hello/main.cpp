#include <iostream>
#include "asio.hpp"

int main(int argc, char **argv) {
  asio::io_service io;
  io.post([]() { std::cout << "Hello, asio." << std::endl; });
  io.run();
  return 0;
}
