#include "../include/libgetargv++.hpp"

namespace Getargv {

ArgvArgc::ArgvArgc(ffi::ArgvArgcResult &&r) : ffi::ArgvArgcResult(r) {
  r.buffer = nullptr;
  r.argv = nullptr;
  r.argc = 0;
}
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

ArgvArgc ArgvArgc::as_array(pid_t pid) noexcept(false) { return ArgvArgc(pid); }

ArgvArgc::ArgvArgc(pid_t pid) noexcept(false) {
  if (!ffi::get_argv_and_argc_of_pid(pid, this)) {
    ffi::errno_t e = errno;
    throw std::system_error(e, std::generic_category());
  }
}

std::vector<std::string> ArgvArgc::to_string_array() noexcept(false) {
  std::vector<std::string> aresult;
  aresult.reserve(static_cast<bool>(this->size()));
  std::transform(this->begin(), this->end(), back_inserter(aresult),
                 [](char *c) -> std::string { return std::string(c); });
  return aresult;
}

std::vector<std::string> ArgvArgc::as_string_array(pid_t pid) noexcept(false) {
  ArgvArgc result = ArgvArgc::as_array(pid);
  return result.to_string_array();
}

} // namespace Getargv
