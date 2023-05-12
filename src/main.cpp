#include <algorithm>
#include <type_traits>
#include <unistd.h>

#include "libgetargv++.hpp"
using Getargv::Argv;
using Getargv::ArgvArgc;

#include <iostream>

int main() {
  pid_t pid = getpid();

  static_assert(std::contiguous_iterator<Argv::Iterator>);
  static_assert(std::contiguous_iterator<ArgvArgc::Iterator>);

  ArgvArgc array = ArgvArgc::as_array(pid);
  for (auto s : array) {
    std::cout << std::string(s) << "\n";
  }
  std::cout << "length: " << array.size() << "\n";
  std::cout << "length: " << std::distance(array.begin(), array.end()) << "\n";

  Argv bytes = Argv::as_bytes(pid);
  for (auto c : bytes) {
    std::cout << c << "\n";
  }

  std::cout << "length: " << bytes.size() << "\n";
  std::cout << "length: " << std::distance(bytes.begin(), bytes.end()) << "\n";

  if (!bytes.print()) {
    std::cerr << "printing fucked up\n";
    return 1;
  }

  return 0;
}
