#include <Block.h>
#include <algorithm>
#include <exception>
#include <iostream>
#include <sys/sysctl.h>
#include <type_traits>
#include <unistd.h>

namespace Getargv::ffi {
  using uint    = ::uint;
  using errno_t = ::errno_t;
} // namespace Getargv::ffi

#include "../../src/argv.cpp" // NOLINT(bugprone-suspicious-include)
#include "../../src/argvargc.cpp" // NOLINT(bugprone-suspicious-include)

#include <criterion/criterion.h>
#include <criterion/hooks.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include <cerrno>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>

struct Child;

auto numPlaces(int n) -> int;
auto randUpTo(int n) -> int;
void redirect_all_std();
auto read_file(FILE* file) -> std::string;
void sig_handler(int sig);
template <typename T, std::size_t N>
auto array_spawn(const char* executable, std::array<T, N> argv) -> Child;
