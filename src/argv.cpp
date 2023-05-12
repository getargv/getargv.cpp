#include <cerrno>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <system_error>

#include "libgetargv++.hpp"

namespace Getargv {

Argv::Argv(ffi::ArgvResult &r) : ffi::ArgvResult(r) {}
Argv::~Argv() { ffi::free_ArgvResult(this); }

bool Argv::print() {
  return ffi::print_argv_of_pid(this->start_pointer, this->end_pointer);
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

Argv Argv::as_bytes(pid_t pid, unsigned int skip, bool nuls) noexcept(false) {
  ffi::ArgvResult result;
  ffi::GetArgvOptions options = {
      .pid = pid, .skip = static_cast<ffi::uint>(skip), .nuls = nuls};
  if (!ffi::get_argv_of_pid(&options, &result)) {
    ffi::errno_t e = errno;
    throw std::system_error(e, std::generic_category());
  }
  return Argv(result);
}
std::string Argv::as_string(pid_t pid, unsigned int skip,
                            bool nuls) noexcept(false) {
  Argv result = Argv::as_bytes(pid, skip, nuls);

  return std::string(result.start_pointer, result.end_pointer);
}

} // namespace Getargv
