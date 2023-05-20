#include <algorithm>
#include <iostream>
#include <type_traits>
#include <unistd.h>

#include "libgetargv++.hpp"
using Getargv::Argv;
using Getargv::ArgvArgc;

int main(int argc, char *argv[]) {
  char **end = argv + argc;
  bool nuls =
      (argc > 1) && std::find_if(argv, end, [](auto c) { return c == "-0"; });

  char **itr = std::find(argv, end, "-s");
  unsigned int skip = (itr != end && ++itr != end) ? static_cast<unsigned int>(std::stoul(*itr)) : 0;

  pid_t pid = (argc > 1) ? std::stoi(argv[argc - 1]) : getpid();

  static_assert(std::contiguous_iterator<Argv::Iterator>);
  static_assert(std::contiguous_iterator<ArgvArgc::Iterator>);

  Argv bytes = Argv::as_bytes(pid, skip, nuls);

  if (!bytes.print()) {
    std::cerr << "printing fucked up\n";
    return 1;
  }

  return 0;
}
