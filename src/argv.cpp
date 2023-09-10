#include "../include/libgetargv++.hpp"

namespace Getargv {

  Argv::Argv(ffi::ArgvResult &&r) : ffi::ArgvResult(r) {
    r.buffer = nullptr;
    r.end_pointer = nullptr;
    r.end_pointer = nullptr;
  }
  Argv::~Argv() { ffi::free_ArgvResult(this); }

  void Argv::print() {
    if (!ffi::print_argv_of_pid(this->start_pointer, this->end_pointer)) {
      ffi::errno_t e = errno;
      throw std::system_error(e, std::generic_category());
    }
  }
  Argv::Iterator Argv::begin() const {
    return Argv::Iterator(this->start_pointer);
  }
  Argv::Iterator Argv::end() const {
    return Argv::Iterator(this->end_pointer + 1);
  }
  ptrdiff_t Argv::size() const {
    return (empty() ? 0 : end_pointer - start_pointer + 1);
  }
  bool Argv::empty() const { return (start_pointer == end_pointer); }

  char &Argv::operator[](const ptrdiff_t index) const {
    auto count = this->size();
    if (count > 0) {
      if (index < 0 && index > (-1 * count)) {
        return end_pointer[index];
      } else if (index >= 0 && index < count) {
        return start_pointer[index];
      }
    }
    throw std::out_of_range("index was out of bounds");
  }

  Argv::Argv(pid_t pid, unsigned int skip, bool nuls) noexcept(false) {
    ffi::GetArgvOptions options = {
      .pid = pid, .nuls = nuls, .skip = static_cast<ffi::uint>(skip)};
    if (!ffi::get_argv_of_pid(&options, this)) {
      ffi::errno_t e = errno;
      throw std::system_error(e, std::generic_category());
    }
  }

  Argv Argv::as_bytes(pid_t pid, unsigned int skip, bool nuls) noexcept(false) {
    return Argv(pid, skip, nuls);
  }

  std::string Argv::to_string() noexcept(false) {
    return std::string(this->start_pointer, this->end_pointer);
  }

  std::string Argv::as_string(pid_t pid, unsigned int skip,
                              bool nuls) noexcept(false) {
    Argv result(pid, skip, nuls);

    return result.to_string();
  }

} // namespace Getargv
