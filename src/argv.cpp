#include "../include/libgetargv++.hpp"

namespace Getargv {

  Argv::Argv(ffi::ArgvResult&& ffiResult) : ffi::ArgvResult(ffiResult) {
    ffiResult.buffer      = nullptr;
    ffiResult.end_pointer = nullptr;
    ffiResult.end_pointer = nullptr;
  }

  Argv::~Argv() { ffi::free_ArgvResult(this); }

  void Argv::print() {
    if (!ffi::print_argv_of_pid(this->start_pointer, this->end_pointer)) {
      const ffi::errno_t savedErrno = errno;
      throw std::system_error(savedErrno, std::generic_category());
    }
  }

  auto Argv::begin() const -> Argv::Iterator {
    return Argv::Iterator(this->start_pointer);
  }

  auto Argv::end() const -> Argv::Iterator {
    return Argv::Iterator(this->end_pointer + 1);
  }

  auto Argv::size() const -> ptrdiff_t {
    return (empty() ? 0 : end_pointer - start_pointer + 1);
  }

  auto Argv::empty() const -> bool {
    return (start_pointer == nullptr || end_pointer == nullptr);
  }

  auto Argv::operator[](const ptrdiff_t index) const -> char& {
    auto count = this->size();
    if (count > 0) {
      if (index < 0 && index > (-1 * count)) {
        return end_pointer[index];
      }
      if (index >= 0 && index < count) {
        return start_pointer[index];
      }
    }
    throw std::out_of_range("index was out of bounds");
  }

  Argv::Argv(pid_t pid, unsigned int skip, bool nuls) noexcept(false) : ArgvResult() {
    ffi::GetArgvOptions const options = {
      #if defined(__cplusplus) && (__cplusplus >= 202002L)
    .skip = static_cast<ffi::uint>(skip),
      .pid  = pid,
      .nuls = nuls,
      #else
      static_cast<ffi::uint>(skip),
      pid,
      nuls,
      #endif
    };
    if (!ffi::get_argv_of_pid(&options, this)) {
      ffi::errno_t const savedErrno = errno;
      throw std::system_error(savedErrno, std::generic_category());
    }
  }

  auto Argv::as_bytes(pid_t pid, unsigned int skip, bool nuls) noexcept(false) -> Argv {
    return Argv(pid, skip, nuls);
  }

  auto Argv::to_string() noexcept(false) -> std::string {
    return { this->start_pointer, this->end_pointer };
  }

  auto Argv::as_string(pid_t pid, unsigned int skip, bool nuls) noexcept(false) -> std::string {
    Argv result(pid, skip, nuls);

    return result.to_string();
  }

} // namespace Getargv
