#include <Block.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace Getargv {
namespace ffi {
typedef ::uint uint;
typedef ::errno_t errno_t;
} // namespace ffi
} // namespace Getargv

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

typedef void (^exit_condition_fn)(void);
static int argmax_mib[] = {CTL_KERN, KERN_ARGMAX};
static int32_t argmax_result = 0;
static size_t argmax_size = sizeof(int32_t);
static char *expected_stdout = NULL;
static char *expected_stderr = NULL;
static exit_condition_fn exit_condition = ^{
  return;
};
static int fail_putchar_unlocked = 0;
static int fail_printf = 0;
static ssize_t fail_write = 0;
static char *fail_strtoimax = NULL;
static int fail_getopt = '\0';
static size_t fail_fwrite = 1;
static bool fail_malloc = false;
static size_t malloc_calls = 0;
static int sysctl_errnos[] = {EFAULT, EINVAL, ENOMEM, ENOTDIR,
                              EISDIR, ENOENT, EPERM};

void __wrap_exit(int status);
int __wrap_sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp,
                  void *newp, size_t newlen);
void *__wrap_malloc(size_t size);
int __wrap_putchar_unlocked(int c);
// int __wrap_printf(const char *restrict format, ...);
/*size_t __wrap_fwrite(const void *restrict ptr, size_t size, size_t nitems,
  FILE *restrict stream);*/
ssize_t __wrap_write(int fildes, const void *buf, size_t nbyte);
/*intmax_t __wrap_strtoimax(const char *restrict str, char **restrict endptr,
  int base);*/
int __wrap_getopt(int argc, char *const argv[], const char *optstring);

int numPlaces(int n);
int randUpTo(int n);
void redirect_all_std(void);
char *inner_strdup(const char *str, size_t size);
std::string read_file(FILE *file);
pid_t inner_spawn(const char *executable, ...);
void sig_handler(int sig);
pid_t array_spawn(const char *executable, char *const *argv);
pid_t varargs_spawn(const char *executable, ...);

#define cr_strdup(str_arg) inner_strdup(str_arg, sizeof(str_arg))
#define spawn(...) varargs_spawn(__VA_ARGS__, NULL)
#define cleanup(callback) __attribute__((__cleanup__(callback)))

struct get_argv_and_argc_of_pid_test_case {
  uint argc;
  const char *argv[5];
};
typedef struct get_argv_and_argc_of_pid_test_case test_case;

void free_strings(criterion_test_params *crp);
void free_double_strings(criterion_test_params *crp);
void free_parse_args(criterion_test_params *crp);
void free_sysctl_return(criterion_test_params *crp);
void free_argv_argc_test_case(criterion_test_params *crp);
void initialize_argv_argc_test_case(test_case *ptr);
void kill_pid(pid_t* pid);
