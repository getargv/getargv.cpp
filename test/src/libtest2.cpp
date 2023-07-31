#include <iostream>

namespace ffi {
#include <unistd.h>
}

#include <libgetargv++.hpp>

using Getargv::ArgvArgc;

int main(int argc, char *argv[]) {

  pid_t pid = ffi::getpid();

  ArgvArgc args(pid);

  for (auto arg : args) {
    std::cout << arg << "\n";
  }

  return 0;
}
