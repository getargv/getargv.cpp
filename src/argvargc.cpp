#include <cerrno>
#include <stdexcept>
#include <string>
#include <system_error>

#include "libgetargv++.hpp"

namespace Getargv {

ArgvArgc::ArgvArgc(ffi::ArgvArgcResult &r) : ffi::ArgvArgcResult(r) {}
ArgvArgc::~ArgvArgc() { ffi::free_ArgvArgcResult(this); }

ptrdiff_t ArgvArgc::size() const { return argc; }
bool ArgvArgc::empty() const { return argc == 0; }
ArgvArgc::Iterator ArgvArgc::begin() const {
  return ArgvArgc::Iterator(&this->argv[0]);
}
ArgvArgc::Iterator ArgvArgc::end() const {
  return ArgvArgc::Iterator(&this->argv[argc]);
}

char *&ArgvArgc::operator[](const ptrdiff_t index) const {
  auto count = this->size();
  if (count > 0) {
    if (index < 0 && index > (-1 * count)) {
      return argv[count + index];
    } else if (index >= 0 && index < count) {
      return argv[index];
    }
  }
  throw std::out_of_range("index was out of bounds");
}

ArgvArgc ArgvArgc::as_array(pid_t pid) noexcept(false) {
  ffi::ArgvArgcResult result;
  if (!ffi::get_argv_and_argc_of_pid(pid, &result)) {
    ffi::errno_t e = errno;
    throw std::system_error(e, std::generic_category());
  }
  return ArgvArgc(result);
}

std::vector<std::string> ArgvArgc::as_string_array(pid_t pid) noexcept(false) {
  ArgvArgc result = ArgvArgc::as_array(pid);
  std::vector<std::string> aresult;
  aresult.reserve(static_cast<bool>(result.size()));
  std::transform(result.begin(), result.end(), back_inserter(aresult),
                 [](auto c) -> std::string { return std::string(c); });
  return aresult;
}

} // namespace Getargv
