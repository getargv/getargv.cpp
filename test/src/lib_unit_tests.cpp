#include "lib_unit_tests.hpp"

using namespace std::string_literals;

ReportHook(TEST_CRASH)(struct criterion_test_stats *stats) {
  // https://github.com/Snaipe/Criterion/blob/master/include/criterion/stats.h#L49-L66
  int signal = stats->signal;
  const char *signame = sys_signame[signal];
  fprintf(stderr, "\33[0;31mSIGNAL\33[0m %s::%s crashed due to signal: %d (%s)",
          stats->test->category, stats->test->name, signal, signame);
  if (signal == SIGABRT) {
    fprintf(stderr,
            " note: abort is the signal used by clang's address sanitizer to "
            "indicate a bad pointer was dereferenced, and while it would "
            "normally print a message, we redirect the stdout/stderr in a LOT "
            "of tests so we aren't likely to see the message.");
  }
  putc('\n', stderr);
}

ReportHook(POST_ALL)(struct criterion_global_stats *stats) {
  if (stats->tests_failed == 0) {
    return;
  }
  fprintf(stderr, "To rerun failed tests: `CRITERION_TEST_PATTERN='*(");
  size_t failedTestIndex = 0;
  struct criterion_suite_stats *suite = stats->suites;
  for (size_t suiteIndex = 0; suiteIndex < stats->nb_suites; suiteIndex++) {
    struct criterion_test_stats *test = suite->tests;
    for (size_t testIndex = 0; testIndex < suite->nb_tests; testIndex++) {
      if (test->test_status == CR_STATUS_FAILED) {
        fprintf(stderr, "%s%s", test->test->data->identifier_,
                (failedTestIndex++ < stats->tests_failed - 1) ? "|" : "");
      }
      test = test->next;
    }
    suite = suite->next;
  }

  fprintf(stderr, ")' make run_lib_unit_tests_verbose`\n");
  fflush(stderr);
}

// Support Stuff

void sig_handler(__attribute__((__unused__)) int sig) {}

int randUpTo(int n) {
  // this is a bad RNG, don't use for anything that matters
  return rand() % n;
}

int numPlaces(int n) {
  if (n < 0) {
    n = (n == INT_MIN) ? INT_MAX : -n;
  }
  if (n < 10) {
    return 1;
  }
  if (n < 100) {
    return 2;
  }
  if (n < 1000) {
    return 3;
  }
  if (n < 10000) {
    return 4;
  }
  if (n < 100000) {
    return 5;
  }
  if (n < 1000000) {
    return 6;
  }
  if (n < 10000000) {
    return 7;
  }
  if (n < 100000000) {
    return 8;
  }
  if (n < 1000000000) {
    return 9;
  }
  /*      2,147,483,647(INT_MAX) is 2^31-1 - add more ifs as needed
          and adjust this final return as well. */
  return 10;
}

void redirect_all_std(void) {
  cr_redirect_stdout();
  cr_redirect_stderr();
}

char *inner_strdup(const char *str, size_t size) {
  char *ptr = (char *)cr_malloc(size);
  if (ptr) {
    memcpy(ptr, str, size);
  }
  return ptr;
}

/*
  This is a bit irresponsible as there's no checking but this is only for
  testing, you have to pass the path to the executable, and then all the
  arguments, argv[0] (usually but not always the name of the executable), all
  arguments must be strings
*/
pid_t varargs_spawn(const char *executable, ...) {
  va_list val;
  const char **args = NULL;
  int argc = 1; // room for terminating NULL

  // Determine number of variadic arguments
  va_start(val, executable);
  while (va_arg(val, const char *) != NULL) {
    argc++;
  }
  va_end(val);
  // cr_log_info("%d args to spawn:\n", argc);
  //  Allocate args, put references to command / variadic arguments + NULL in
  //  args because last val is NULL, and we malloc'd an extra pointer, we're
  //  good
  args = (const char **)malloc(argc * sizeof(char *));
  va_start(val, executable);
  for (int i = 0; i < argc; i++) {
    args[i] = va_arg(val, const char *);
    // cr_log_info("%s\n", args[i]);
  }
  va_end(val);

#pragma clang diagnostic push
#pragma clang diagnostic ignored                                               \
    "-Wincompatible-pointer-types-discards-qualifiers"
  return array_spawn(executable, (char *const *)args);
#pragma clang diagnostic pop

  _Exit(EXIT_FAILURE);
}

pid_t array_spawn(const char *executable, char *const *argv) {
  sigset_t emptymask = 0;
  sigemptyset(&emptymask);
  struct sigaction act;
  act.sa_handler = sig_handler;
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGCONT); // NOLINT(hicpp-signed-bitwise)
  act.sa_flags = 0;
  sigaction(SIGCONT, &act, NULL);

  pid_t pid = fork();
  if (pid != 0) {
    sigsuspend(&emptymask); // wait for child to be ready
    return pid;
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored                                               \
    "-Wincompatible-pointer-types-discards-qualifiers"
  execv(executable, argv);
#pragma clang diagnostic pop
  perror("child");
  _Exit(EXIT_FAILURE);
}

void initialize_argv_argc_test_case(
    struct get_argv_and_argc_of_pid_test_case *ptr) {
  ptr->argc = 0;
  for (size_t i = 0; i < 5; i++) {
    ptr->argv[i] = NULL;
  }
}

ParameterizedTestParameters(argv_argc, successful) {
  size_t nb_params = 7;

  struct get_argv_and_argc_of_pid_test_case *params =
      (get_argv_and_argc_of_pid_test_case *)cr_malloc(
          sizeof(struct get_argv_and_argc_of_pid_test_case) * nb_params);

  char *empt = cr_strdup("");
  char *oneA = cr_strdup("a");

  for (size_t i = 0; i < nb_params; i++) {
    initialize_argv_argc_test_case(&params[i]);
  }

  params[1].argc = 1;
  params[1].argv[0] = empt;

  params[2].argc = 2;
  params[2].argv[0] = empt;
  params[2].argv[1] = empt;

  params[3].argc = 1;
  params[3].argv[0] = oneA;

  params[4].argc = 2;
  params[4].argv[0] = oneA;
  params[4].argv[1] = empt;

  params[5].argc = 2;
  params[5].argv[0] = oneA;
  params[5].argv[1] = oneA;

  params[6].argc = 2;
  params[6].argv[0] = empt;
  params[6].argv[1] = oneA;

  return cr_make_param_array(get_argv_and_argc_of_pid_test_case, params,
                             nb_params, free_argv_argc_test_case);
}

ParameterizedTest(get_argv_and_argc_of_pid_test_case *param, argv_argc,
                  successful) {

  pid_t pid = array_spawn("bin/child", (char *const *)param->argv);
  cr_assert_neq(pid, -1);

  cr_assert_no_throw(
      {
        Getargv::ArgvArgc results(pid);
        cr_assert_eq(results.size(), param->argc);
        cr_assert_eq(results.empty(), param->argc == 0);
        for (int i = 0; i < param->argc; i++) {
          cr_expect_str_eq(results[i], param->argv[i],
                           "#%d: actual='%s' expected='%s'", param->argc,
                           results[i], param->argv[i]);

          int index = (i + 1) * -1;
          std::string actual = results[index];
          std::string expected = param->argv[(int)param->argc + index];
          cr_expect_eq(actual, expected, "actual='%s' expected='%s'",
                       actual.c_str(), expected.c_str());
        }
        size_t i = 0;
        for (auto arg : results) {
          cr_expect_str_eq(arg, param->argv[i],
                           "#%d: actual='%s' expected='%s'", param->argc, arg,
                           param->argv[i]);
          i++;
        }
      },
      std::system_error);
  kill(pid, SIGTERM); // signal to child to be done
}

ParameterizedTestParameters(print_argv_of_pid, successful) {
  size_t nb_params = 6;

  get_argv_and_argc_of_pid_test_case *params =
      (get_argv_and_argc_of_pid_test_case *)cr_malloc(
          sizeof(get_argv_and_argc_of_pid_test_case) * nb_params);

  initialize_argv_argc_test_case(&params[0]);
  params[0].argv[0] = cr_strdup("print me");
  params[0].argc = 1;

  initialize_argv_argc_test_case(&params[1]);
  params[1].argv[0] = cr_strdup("one");
  params[1].argv[1] = cr_strdup("two");
  params[1].argc = 2;

  initialize_argv_argc_test_case(&params[2]);

  initialize_argv_argc_test_case(&params[3]);
  params[3].argv[0] = cr_strdup("");
  params[3].argv[1] = cr_strdup("");
  params[3].argc = 2;

  initialize_argv_argc_test_case(&params[4]);
  params[4].argv[0] = cr_strdup("");
  params[4].argv[1] = cr_strdup("");
  params[4].argv[2] = cr_strdup("");
  params[4].argc = 3;

  initialize_argv_argc_test_case(&params[5]);
  params[5].argv[0] = cr_strdup("");
  params[5].argc = 1;

  return cr_make_param_array(get_argv_and_argc_of_pid_test_case, params,
                             nb_params, free_argv_argc_test_case);
}

ParameterizedTest(get_argv_and_argc_of_pid_test_case *param, print_argv_of_pid,
                  successful, .init = cr_redirect_stdout) {
  pid_t pid = array_spawn("bin/child", (char *const *)param->argv);
  cr_assert_neq(pid, -1);

  cr_assert_no_throw(
      {
        try {
          Getargv::Argv args(pid);
          cr_assert(args.print());
          fflush(stdout);
        } catch (std::system_error e) {
          cr_log_error("'%s'[%d] error %d:'%s'",
                       param->argc > 0 ? param->argv[0] : "empty arg array",
                       param->argc, e.code().value(), e.what());
          throw e;
        }

        FILE *f_stdout = cr_get_redirected_stdout();
        size_t size = 1024 * 8;
        char buffer[size];
        char *head = buffer;
        size_t read = 0;

        do {
          read = fread(head, 1, size - (head - buffer), f_stdout);
          head += read;
        } while (read > 0 && (head - buffer) < size);
        std::string actual(buffer, (head - buffer));

        std::string expected;
        for (int i = 0; i < param->argc; i++) {
          auto a = param->argv[i];
          expected += a;
          expected += "\0"s;
        }

        cr_assert_eq(actual, expected,
                     "actual: '%.*s'[%ld] != expected: '%.*s'[%ld]",
                     (int)actual.size(), actual.c_str(), actual.size(),
                     (int)expected.size(), expected.c_str(), expected.size());
      },
      std::system_error, "error thrown");
}

Test(argv_argc, to_string_array) {
  pid_t pid = spawn("bin/child", "bin/child");
  cr_assert_neq(pid, -1);

  cr_assert_no_throw(
      {
        Getargv::ArgvArgc results(pid);
        std::vector<std::string> a = results.to_string_array();
      },
      std::system_error, "error thrown");
}

Test(argv_argc, fail_find_procargs) {
  cr_assert_throw({ Getargv::ArgvArgc result(-1); }, std::system_error);
}

Test(argv_argc, fail_perm_procargs) {
  cr_assert_throw({ Getargv::ArgvArgc result(0); }, std::system_error);
}

Test(argv, simple) {
  std::string expected = "bin/child\0"s;
  pid_t pid = spawn(expected.c_str(), expected.c_str());

  cr_expect_no_throw(
      {
        Getargv::Argv proc_ptrs(pid, 0, true);
        cr_assert_eq(proc_ptrs.size(), expected.size());
        std::string actual(proc_ptrs.begin(), proc_ptrs.end());
        cr_assert_eq(actual, expected);
      },
      std::system_error,
      "Argv constructor threw an exception when it shouldn't have");

  kill(pid, SIGTERM); // signal to child to be done
}

Test(argv, nuls_false) {
  std::string expected = "one\0two\0three\0"s;

  pid_t pid = spawn("bin/child", "one", "two", "three");

  cr_expect_no_throw(
      {
        Getargv::Argv proc_ptrs(pid, 0, false);
        cr_assert_eq(proc_ptrs.size(), expected.size());
        std::string actual(proc_ptrs.begin(), proc_ptrs.end());
        cr_assert_eq(actual, expected);
      },
      std::system_error,
      "Argv constructor threw an exception when it shouldn't have");

  kill(pid, SIGTERM); // signal to child to be done
}

Test(argv, nuls_true) {

  std::string expected = "bin/tests --verbose 2 -j1\0"s;

  pid_t pid = spawn("bin/child", "bin/tests", "--verbose", "2", "-j1");

  cr_expect_no_throw(
      {
        Getargv::Argv proc_ptrs(pid, 0, true);
        cr_assert_eq(proc_ptrs.size(), expected.size());
        std::string actual(proc_ptrs.begin(), proc_ptrs.end());
        cr_assert_eq(actual, expected);
      },
      std::system_error);
  kill(pid, SIGTERM); // signal to child to be done
}

Test(argv, skip_one) {
  cr_assert_no_throw(
      {
        Getargv::Argv proc_ptrs(getppid(), 1, false);

        cr_assert_neq(proc_ptrs.size(), 0);
        cr_expect_eq(*--proc_ptrs.end(), '\0'); // end of args to print
        cr_expect_lt(proc_ptrs.begin(), proc_ptrs.end()); // whole buffer

        std::string buf1 = "--verbose\0002\0-j1"s;
        std::string buf2 = "-j0\0"s;
        std::string expected = (criterion_options.jobs > 0) ? buf1 : buf2;
        std::string actual(proc_ptrs.begin(), proc_ptrs.end());
        cr_assert_eq(actual, expected, "'%.*s'[%ld] != '%.*s'[%ld]",
                     (int)actual.size(), actual.c_str(), actual.size(),
                     (int)expected.size(), expected.c_str(), expected.size());
      },
      std::system_error);
}

Test(argv, skip_all) {
  int skip = (criterion_options.jobs > 0) ? 4 : 2;

  cr_assert_no_throw(
      {
        Getargv::Argv proc_ptrs(getppid(), skip, false);

        cr_expect_eq(proc_ptrs.size(), 0);
      },
      std::system_error);
}

Test(argv, skip_too_many) {
  errno = 0;
  cr_assert_throw({ Getargv::Argv proc_ptrs(getpid(), 5, true); },
                  std::system_error);
}

Test(argv, permissions) {
  errno = 0;
  cr_assert_throw({ Getargv::Argv proc_ptrs(1, 5, true); }, std::system_error);
}

Test(argv, not_exists) {
  cr_assert_throw({ Getargv::Argv proc_ptrs(-1, 5, true); }, std::system_error);
}

Test(argv, malloc_fail) {
  fail_malloc = true;

  cr_assert_throw({ Getargv::Argv proc_ptrs(-1, 5, true); }, std::system_error);
}

void free_strings(struct criterion_test_params *crp) {
  char **strings = (char **)crp->params;
  for (size_t i = 0; i < crp->length; ++i) {
    cr_free(strings[i]);
  }
  cr_free(strings);
}

void free_argv_argc_test_case(struct criterion_test_params *crp) {
  struct get_argv_and_argc_of_pid_test_case *params =
      (struct get_argv_and_argc_of_pid_test_case *)crp->params;
  for (size_t i = 0; i < crp->length; ++i) {
    cr_free(params[i].argv);
  }
  cr_free(params);
}
