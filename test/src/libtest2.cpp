#include <iostream>

namespace ffi {
#include <unistd.h>
} //namespace ffi

#include <libgetargv++.hpp>

using Getargv::ArgvArgc;

auto main(int /*argc*/, char* /*argv*/[]) -> int {
  const pid_t pid = ffi::getpid();

  const ArgvArgc args(pid);

  for (auto* arg : args) {
    std::cout << arg << "\n";
  }

  return 0;
}
