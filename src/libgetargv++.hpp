#include "iter.hpp"
#include <cstddef>

#ifndef LIBGETARGVPLUSPLUS_H
#define LIBGETARGVPLUSPLUS_H

namespace ffi {
#include <libgetargv.h>
}

/* TODO:
  - tests
 */

namespace Getargv {

struct Argv : protected ffi::ArgvResult {
  using Iterator = Iterator<char>;

  static Argv as_bytes(pid_t pid, unsigned int skip = 0,
                       bool nuls = false) noexcept(false);
  static std::string as_string(pid_t pid, unsigned int skip,
                               bool nuls) noexcept(false);
  Argv(Argv &r) = delete;
  Argv(ffi::ArgvResult &r);
  ~Argv();

  char &operator[](const ptrdiff_t index) const;
  ptrdiff_t size() const;
  bool empty() const;
  Iterator begin() const;
  Iterator end() const;

  bool print();
};

struct ArgvArgc : protected ffi::ArgvArgcResult {
  using Iterator = Iterator<char *>;

  static ArgvArgc as_array(pid_t pid) noexcept(false);
  static std::vector<std::string> as_string_array(pid_t pid) noexcept(false);
  ArgvArgc(ArgvArgc &r) = delete;
  ArgvArgc(ffi::ArgvArgcResult &r);
  ~ArgvArgc();

  char *&operator[](const ptrdiff_t index) const;
  ptrdiff_t size() const;
  bool empty() const;
  Iterator begin() const;
  Iterator end() const;
};

} // namespace Getargv
#endif
