#include <Block.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace Getargv::ffi {
  using uint    = ::uint;
  using errno_t = ::errno_t;
} // namespace Getargv::ffi

#include "../../src/argv.cpp"
#include "../../src/argvargc.cpp"

#include <criterion/criterion.h>
#include <criterion/hooks.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include <cerrno>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

auto numPlaces(int n) -> int;
auto randUpTo(int n) -> int;
void redirect_all_std();
auto inner_strdup(const char* str, size_t size) -> char*;
auto read_file(FILE* file) -> std::string;
auto inner_spawn(const char* executable, ...) -> pid_t;
void sig_handler(int sig);
auto array_spawn(const char* executable, char* const* argv) -> pid_t;
auto varargs_spawn(const char* executable, ...) -> pid_t;

#define cr_strdup(str_arg) inner_strdup(str_arg, sizeof(str_arg))
#define spawn(...)         varargs_spawn(__VA_ARGS__, NULL)
#define cleanup(callback)  __attribute__((__cleanup__(callback)))

struct get_argv_and_argc_of_pid_test_case {
  uint        argc;
  const char* argv[5];
};
using test_case = struct get_argv_and_argc_of_pid_test_case;

void free_strings(criterion_test_params* crp);
void free_double_strings(criterion_test_params* crp);
void free_parse_args(criterion_test_params* crp);
void free_sysctl_return(criterion_test_params* crp);
void free_argv_argc_test_case(criterion_test_params* crp);
void initialize_argv_argc_test_case(test_case* ptr);
void kill_pid(const pid_t* pid);
