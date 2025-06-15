#include "lib_unit_tests.hpp"

using namespace std::string_literals;
using cr_string = std::basic_string<char, std::char_traits<char>, criterion::allocator<char>>;

ReportHook(TEST_CRASH)(struct criterion_test_stats* stats) {
  // https://github.com/Snaipe/Criterion/blob/master/include/criterion/stats.h#L49-L66
  const int   signal  = stats->signal;
  const char* signame = sys_signame[signal]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
  std::cerr << "\33[0;31mSIGNAL\33[0m " << stats->test->category << "::" << stats->test->name << " crashed due to signal: " << signal << " (" << signame << ")";
  if (signal == SIGABRT) {
    std::cerr << " note: abort is the signal used by clang's address sanitizer to "
                "indicate a bad pointer was dereferenced, and while it would "
                "normally print a message, we redirect the stdout/stderr in a LOT "
                "of tests so we aren't likely to see the message.";
  }
  std::cerr << '\n';
}

ReportHook(POST_ALL)(struct criterion_global_stats* stats) {
  if (stats->tests_failed == 0) {
    return;
  }
  std::cerr << "To rerun failed tests: `CRITERION_TEST_PATTERN='*(";
  size_t                        failedTestIndex = 0;
  std::vector<const char*>      printed;
  struct criterion_suite_stats* suite = stats->suites;
  for (size_t suiteIndex = 0; suiteIndex < stats->nb_suites; suiteIndex++) {
    struct criterion_test_stats* test = suite->tests;
    for (size_t testIndex = 0; testIndex < suite->nb_tests; testIndex++) {
      if (test->test_status == CR_STATUS_FAILED) {
        if (!std::ranges::any_of(printed, [test](const char* element) { return strcmp(element, test->test->data->identifier_) == 0; })) {
          if (failedTestIndex > 0) { std::cerr << "|"; }
          std::cerr << test->test->data->identifier_;
          printed.push_back(test->test->data->identifier_);
        }
        failedTestIndex++;
        ;
      }
      test = test->next;
    }
    suite = suite->next;
  }

  std::cerr << ")' make run_lib_unit_tests_verbose`\n";
  (void)fflush(stderr);
}

// Support Stuff

void sig_handler(__attribute__((__unused__)) int sig) {}

auto randUpTo(int n) -> int {
  // this is a bad RNG, don't use for anything that matters
  return rand() % n; // NOLINT(cert-msc30-c,cert-msc50-cpp,concurrency-mt-unsafe)
}

void redirect_all_std() {
  cr_redirect_stdout();
  cr_redirect_stderr();
}

struct Child {
  pid_t pid; // NOLINT(misc-non-private-member-variables-in-classes)
  explicit Child(pid_t pid) : pid(pid) {}
  Child(const Child& other)       = delete;
  Child(const Child&& other)      = delete;
  auto operator=(Child&& other) -> Child& = delete;
  auto operator=(Child& other) -> Child& = delete;
  ~Child() {
    kill(this->pid, SIGTERM); // signal to child to be done
  }
};

template <std::size_t N>
static auto array_spawn(char const* executable, std::array<char const*, N> argv) -> Child {
  sigset_t emptymask = 0;
  sigemptyset(&emptymask);
  struct sigaction act {};
  act.sa_handler = sig_handler; // NOLINT(cppcoreguidelines-pro-type-union-access)
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGCONT); // NOLINT(hicpp-signed-bitwise)
  act.sa_flags = 0;
  sigaction(SIGCONT, &act, nullptr);

  const pid_t pid = fork();
  if (pid != 0) {
    // wait for child to be ready
    sigsuspend(&emptymask); // NOLINT(concurrency-mt-unsafe)
    cr_assert(ne(pid, -1));
    return Child(pid);
  }

  execv(executable, const_cast<char* const*>(argv.data())); // NOLINT(cppcoreguidelines-pro-type-const-cast)

  perror("child");
  _Exit(EXIT_FAILURE);
}

template <std::size_t N>
static auto array_spawn(char const* executable, std::array<cr_string, N> argv) -> Child {
  std::array<const char*, N> result_array = {};
  for (size_t i = 0; i < N; i++) {
    result_array.at(i) = argv.at(i).c_str();
  }
  return array_spawn(executable, result_array);
}

/*
  This is a bit irresponsible as there's no checking but this is only for
  testing, you have to pass the path to the executable, and then all the
  arguments, argv[0] (usually but not always the name of the executable), all
  arguments must be strings
 */
template <typename... Ts>
static auto spawn(char const* executable, Ts... args) -> Child {
  // Determine number of variadic arguments
  const uint                          size  = sizeof...(args) + 1;
  const std::array<char const*, size> array = { args..., nullptr };

  return array_spawn(executable, array);

  _Exit(EXIT_FAILURE);
}

using array2 = std::array<cr_string, 2>;
ParameterizedTestParameters(argv_argc, successful) {
  static criterion::parameters<array2> params;

  const cr_string empt;
  const cr_string oneA = "a";

  params.push_back({});
  params.push_back({ empt });
  params.push_back({ empt, empt });
  params.push_back({ oneA });
  params.push_back({ oneA, empt });
  params.push_back({ oneA, oneA });
  params.push_back({ empt, oneA });

  return params;
}

ParameterizedTest(array2* param, argv_argc, successful) {
  Child const child = array_spawn("bin/child", *param);

  //  cr_assert(not(throw(std::system_error, {
  const Getargv::ArgvArgc results(child.pid);
  cr_expect(eq(results.size(), param->size()));
  cr_expect(eq(results.empty(), param->empty()));
  for (int i = 0; i < param->size(); i++) {
    cr_expect(eq(results[i], param->at(i)));
    const int         index    = (i + 1) * -1;
    const std::string actual   = results[index];
    const cr_string   crexp    = param->at((int)param->size() + index);
    const std::string expected = std::string(crexp.begin(), crexp.end());
    cr_expect(eq(actual, expected));
  }
  size_t index = 0;
  for (auto* arg : results) {
    cr_expect(eq(arg, param->at(index)));
    index++;
  }
  //  })));
}

using array3 = std::array<cr_string, 3>;
ParameterizedTestParameters(print_argv_of_pid, successful) {
  static criterion::parameters<array3> params;

  const cr_string print("print me");
  const cr_string one("one");
  const cr_string two("two");
  const cr_string empty;

  params.push_back({ print });
  params.push_back({ one, two });
  params.push_back({});
  params.push_back({ empty, empty });
  params.push_back({ empty, empty, empty });
  params.push_back({ empty });

  return params;
}

ParameterizedTest(array3* param, print_argv_of_pid, successful, .init = cr_redirect_stdout) {
  Child const child = array_spawn("bin/child", *param);

  cr_assert(not(throw(std::system_error, {
    Getargv::Argv(child.pid).print();
    (void)fflush(stdout);
  })));

  std::string expected;
  for (auto& arg : *param) {
    expected += arg;
    expected += "\0"s;
  }

  auto&       f_cout = criterion::get_redirected_cout();
  std::string actual(std::istreambuf_iterator<char>(f_cout), {});
  cr_assert(eq(actual, expected));
  //cr_assert_stdout_eq_str(expected.c_str());
}

Test(print_argv_of_pid, failure) {
  std::string const                empty;
  const std::array<const char*, 2> argv  = { empty.c_str(), nullptr };
  Child const                      child = array_spawn("bin/child", argv);
  // NOLINTBEGIN(cppcoreguidelines-owning-memory)
  (void)fclose(stdout);
  // NOLINTEND(cppcoreguidelines-owning-memory)
  cr_assert(throw(std::system_error, Getargv::Argv(child.pid).print()));
}

using array1 = std::array<cr_string, 1>;
ParameterizedTestParameters(argv_of_pid_empty, correct) {
  static criterion::parameters<array1> params;

  const cr_string notempty("not empty");
  const cr_string empty;

  params.push_back({ notempty });
  params.push_back({ empty });
  params.push_back({});

  return params;
}

ParameterizedTest(array1* param, argv_of_pid_empty, correct) {
  Child const child = array_spawn("bin/child", *param);

  cr_assert(eq(Getargv::Argv::as_bytes(child.pid).empty(), param->empty()));
}

Test(argv_of_pid_indexing, works) {
  const std::string          expected      = "abcdefghijklmnopqrstuvwxyz";
  const size_t               expected_size = expected.size();
  std::array<const char*, 2> argv          = { expected.c_str(), nullptr };
  Child const                child         = array_spawn("bin/child", argv);

  const Getargv::Argv args(child.pid);
  for (int i = 0; i < expected_size; i++) {
    cr_assert(eq(args[i], argv[0][i]));
  }
  for (int i = 1; i <= expected_size; i++) {
    cr_assert(eq(args[static_cast<ptrdiff_t>(i * -1)], argv[0][expected_size - i]));
  }
}

Test(argv_of_pid_indexing, failure) {
  const std::array<const char*, 2> argv  = { "", nullptr };
  Child const                      child = array_spawn("bin/child", argv);

  const Getargv::Argv args(child.pid);
  cr_assert(throw(std::out_of_range, args[100000]));
}

Test(argv_argc_of_pid_indexing, failure) {
  const std::array<const char*, 2> argv  = { "", nullptr };
  Child const                     child = array_spawn("bin/child", argv);

  const Getargv::ArgvArgc args(child.pid);
  cr_assert(throw(std::out_of_range, args[100000]));
}

Test(argv_as_string, works) {
  const char*                      expected = "abcdefghijklmnopqrstuvwxyz";
  const std::array<const char*, 2>  argv     = { expected, nullptr };
  Child const                      child    = array_spawn("bin/child", argv);

  const auto actual = Getargv::Argv::as_string<std::string>(child.pid);
  cr_assert(eq(actual, expected));
}

Test(argv_argc, to_string_array) {
  const Child child = spawn("bin/child", "bin/child");

  cr_assert(not(throw(std::system_error, {
    const Getargv::ArgvArgc results(child.pid);
    const auto             array = results.to_vector<std::string>();
  })),
            "error thrown");
}

Test(argv_argc, to_string_view_array) {
  const Child child = spawn("bin/child", "bin/child");

  cr_assert(not(throw(std::system_error, {
    const Getargv::ArgvArgc results(child.pid);
    const auto              array = results.to_vector<std::string_view>();
  })),
            "error thrown");
}

Test(argv_argc, as_string_array) {
  Child const child = spawn("bin/child", "bin/child");

  cr_assert(not(throw(std::system_error, {
    Getargv::ArgvArgc::as_vector<std::string>(child.pid);
  })),
            "error thrown");
}

Test(argv, convert_from_ffi_type) {
  auto func = []() -> Getargv::ffi::ArgvResult {
    return {};
  };
  Getargv::Argv const array(func());
}
Test(argv_argc, convert_from_ffi_type) {
  auto func = []() -> Getargv::ffi::ArgvArgcResult {
    return {};
  };
  Getargv::ArgvArgc const array(func());
}

Test(argv_argc, fail_find_procargs) {
  cr_assert(throw(std::system_error, { Getargv::ArgvArgc::as_array(-1); }));
}

Test(argv_argc, fail_perm_procargs) {
  cr_assert(throw(std::system_error, { Getargv::ArgvArgc::as_array(0); }));
}

Test(argv_argc, not_copyable) {
  cr_assert(not(std::is_copy_assignable_v<Getargv::ArgvArgc>));
  cr_assert(not(std::is_copy_constructible_v<Getargv::ArgvArgc>));
  cr_assert(not(std::is_nothrow_copy_assignable_v<Getargv::ArgvArgc>));
  cr_assert(not(std::is_nothrow_copy_constructible_v<Getargv::ArgvArgc>));
  cr_assert(not(std::is_trivially_copy_assignable_v<Getargv::ArgvArgc>));
  cr_assert(not(std::is_trivially_copy_constructible_v<Getargv::ArgvArgc>));
  cr_assert(not(std::is_trivially_copyable_v<Getargv::ArgvArgc>));
}

Test(argv, simple) {
  const std::string expected = "bin/child\0"s;
  Child const       child    = spawn(expected.c_str(), expected.c_str());

  //  cr_expect(not(throw(std::system_error, {
  const Getargv::Argv proc_ptrs(child.pid, 0, true);
  cr_expect(eq(proc_ptrs.size(), expected.size()));
  const std::string actual(proc_ptrs.begin(), proc_ptrs.end());
  cr_assert(eq(actual, expected));
  //  })), "Argv constructor threw an exception when it shouldn't have");
}

Test(argv, nuls_false) {
  const std::string expected = "one\0two\0three\0"s;

  const Child child = spawn("bin/child", "one", "two", "three");

  //  cr_expect(not(throw(std::system_error, {
  const Getargv::Argv proc_ptrs(child.pid, 0, false);
  cr_expect(eq(proc_ptrs.size(), expected.size()));
  const std::string actual(proc_ptrs.begin(), proc_ptrs.end());
  cr_assert(eq(actual, expected));
  //  })), "Argv constructor threw an exception when it shouldn't have");
}

Test(argv, nuls_true) {
  const std::string expected = "bin/tests --verbose 2 -j1\0"s;
  const Child       child    = spawn("bin/child", "bin/tests", "--verbose", "2", "-j1");

  //  cr_expect(not(throw(std::system_error, {
  const Getargv::Argv proc_ptrs(child.pid, 0, true);
  cr_expect(eq(proc_ptrs.size(), expected.size()));
  const std::string actual(proc_ptrs.begin(), proc_ptrs.end());
  cr_assert(eq(actual, expected));
  //  })));
}

Test(argv, skip_one) {
  //  cr_assert(not(throw(std::system_error, {
  const Getargv::Argv proc_ptrs(getppid(), 1, false);

  cr_expect(ne(proc_ptrs.size(), 0));
  char actual_char = *--proc_ptrs.end();
  cr_expect(eq(actual_char, '\0')); // end of args to print
  uintptr_t begin = *proc_ptrs.begin();
  uintptr_t end   = *proc_ptrs.end();
  cr_expect(lt(begin, end)); // whole buffer

  const std::string buf1     = "--verbose\0002\0-j1"s;
  const std::string buf2     = "-j0\0"s;
  const std::string expected = (criterion_options.jobs > 0) ? buf1 : buf2;
  const std::string actual(proc_ptrs.begin(), proc_ptrs.end());
  cr_assert(eq(actual, expected));
  //  })));
}

Test(argv, skip_all) {
  const int skip = (criterion_options.jobs > 0) ? 4 : 2;

  //  cr_assert(not(throw(std::system_error, {
  const Getargv::Argv proc_ptrs(getppid(), skip, false);

  cr_expect(eq(proc_ptrs.size(), 0));
  //  })));
}

Test(argv, skip_too_many) {
  errno = 0;
  cr_assert(throw(std::system_error, { const Getargv::Argv proc_ptrs(getpid(), 5, true); }));
}

Test(argv, permissions) {
  errno = 0;
  cr_assert(throw(std::system_error, { const Getargv::Argv proc_ptrs(1, 5, true); }));
}

Test(argv, not_exists) {
  cr_assert(throw(std::system_error, { const Getargv::Argv proc_ptrs(-1, 5, true); }));
}

Test(argv, not_copyable) {
  cr_assert(not(std::is_copy_assignable_v<Getargv::Argv>));
  cr_assert(not(std::is_copy_constructible_v<Getargv::Argv>));
  cr_assert(not(std::is_nothrow_copy_assignable_v<Getargv::Argv>));
  cr_assert(not(std::is_nothrow_copy_constructible_v<Getargv::Argv>));
  cr_assert(not(std::is_trivially_copy_assignable_v<Getargv::Argv>));
  cr_assert(not(std::is_trivially_copy_constructible_v<Getargv::Argv>));
  cr_assert(not(std::is_trivially_copyable_v<Getargv::Argv>));
}
