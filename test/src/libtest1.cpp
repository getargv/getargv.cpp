#include <algorithm>
#include <iostream>
#include <type_traits>

namespace ffi {
#include <unistd.h>
}

#include <libgetargv++.hpp>

using Getargv::Argv;
using Getargv::ArgvArgc;

int main(int argc, char *argv[]) {
  char **end = argv + argc;
  bool nuls = (argc > 1) && std::find_if(argv, end, [](char *c) {
                return std::string(c) == "-0";
              });

  char **itr = std::find(argv, end, "-s");
  unsigned int skip = (itr != end && ++itr != end)
                          ? static_cast<unsigned int>(std::stoul(*itr))
                          : 0;

  pid_t pid = (argc > 1) ? std::stoi(argv[argc - 1]) : ffi::getpid();

#if defined(__cplusplus) && (__cplusplus == 202002L)
  static_assert(std::contiguous_iterator<Argv::Iterator>);
  static_assert(std::contiguous_iterator<ArgvArgc::Iterator>);
#else
  static_assert(
      std::is_same<std::random_access_iterator_tag,
                   Argv::Iterator::iterator_category>::value,
      "testing that Argv::Iterator is a random_access_iterator failed");
  static_assert(
      std::is_same<std::random_access_iterator_tag,
                   ArgvArgc::Iterator::iterator_category>::value,
      "testing that ArgvArgc::Iterator is a random_access_iterator failed");
#endif

  Argv bytes = Argv::as_bytes(pid, skip, nuls);

  if (!bytes.print()) {
    std::cerr << "printing fucked up\n";
    return 1;
  }

  return 0;
}
