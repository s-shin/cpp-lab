#include <iostream>
#include <functional>

class SafeReturnInt : public std::function<int()> {
public:
  typedef std::function<int()> func;

  SafeReturnInt() : func() {}

  template <class F>
  SafeReturnInt(F f) : func(f) {}

  int operator()() const {
    if (!*this) return 0; // eval operator bool
    return function::operator()();
  }
};

int main(int argc, char *argv[]) {
  SafeReturnInt foo; // not initialized
  std::cout << foo() << std::endl;
  foo = []() { return 100; };
  std::cout << foo() << std::endl;
  return 0;
}
