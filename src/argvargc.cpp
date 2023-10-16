#include "../include/libgetargv++.hpp"
#include <algorithm>

namespace Getargv {

  ArgvArgc::ArgvArgc(ffi::ArgvArgcResult&& ffiResult) : ffi::ArgvArgcResult(ffiResult) {
    ffiResult.buffer = nullptr;
    ffiResult.argv   = nullptr;
    ffiResult.argc   = 0;
  }

  ArgvArgc::~ArgvArgc() { ffi::free_ArgvArgcResult(this); }

  auto ArgvArgc::size() const -> ptrdiff_t { return argc; }

  auto ArgvArgc::empty() const -> bool { return argc == 0; }

  auto ArgvArgc::begin() const -> ArgvArgc::Iterator {
    return ArgvArgc::Iterator(&this->argv[0]);
  }

  auto ArgvArgc::end() const -> ArgvArgc::Iterator {
    return ArgvArgc::Iterator(&this->argv[argc]);
  }

  auto ArgvArgc::operator[](const ptrdiff_t index) const -> char*& {
    auto count = this->size();
    if (count > 0) {
      if (index < 0 && index >= (-1 * count)) {
        return argv[count + index];
      }
      if (index >= 0 && index < count) {
        return argv[index];
      }
    }
    throw std::out_of_range("index was out of bounds");
  }

  auto ArgvArgc::as_array(pid_t pid) noexcept(false) -> ArgvArgc { return ArgvArgc(pid); }

  ArgvArgc::ArgvArgc(pid_t pid) noexcept(false) : ArgvArgcResult() {
    if (!ffi::get_argv_and_argc_of_pid(pid, this)) {
      ffi::errno_t const savedErrno = errno;
      throw std::system_error(savedErrno, std::generic_category());
    }
  }

  auto ArgvArgc::to_string_array() const noexcept(false) -> std::vector<std::string> {
    std::vector<std::string> aresult;
    aresult.reserve(static_cast<size_t>(this->size()));
    std::transform(this->begin(), this->end(), back_inserter(aresult), [](char* cStr) -> std::string { return { cStr }; });
    return aresult;
  }

  auto ArgvArgc::as_string_array(pid_t pid) noexcept(false) -> std::vector<std::string> {
    const ArgvArgc result = ArgvArgc::as_array(pid);
    return result.to_string_array();
  }

} // namespace Getargv
