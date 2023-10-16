#include <algorithm>
#include <iostream>
#include <system_error>
#include <type_traits>

namespace ffi {
#include <unistd.h>
} // namespace ffi

#include <libgetargv++.hpp>

using Getargv::Argv;
using Getargv::ArgvArgc;

auto main(int argc, char* argv[]) -> int {
  char**     end  = argv + argc;
  const bool nuls = (argc > 1) && std::find_if(argv, end, [](char* cStr) {
    return std::string(cStr) == "-0";
  }) != end;

  char**             itr  = std::find(argv, end, "-s");
  const unsigned int skip = (itr != end && ++itr != end)
                            ? static_cast<unsigned int>(std::stoul(*itr))
                            : 0;

  const pid_t pid = (argc > 1) ? std::stoi(argv[argc - 1]) : ffi::getpid();

#if defined(__cplusplus) && (__cplusplus >= 202002L)
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

  try {
    bytes.print();
  } catch (std::system_error& _unused) {
    std::cerr << "printing fucked up\n";
    return 1;
  }

  return 0;
}
