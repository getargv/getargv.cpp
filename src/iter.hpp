#ifndef ITERGEN
#define ITERGEN

#include <iterator>
namespace IterGen {

template <typename T> struct Iterator {
#if defined(__cplusplus) && (__cplusplus == 202002L)
  using iterator_concept [[maybe_unused]] = std::contiguous_iterator_tag;
#else
  // strongest type of iterator in c++17
  using iterator_category = std::random_access_iterator_tag;
#endif
  using difference_type = std::ptrdiff_t;
  using element_type = T;
  // using value_type = element_type; // ← compiler doesn't seem to need this,
  // but everyone online says it's needed…
  using pointer = element_type *;
  using reference = element_type &;

  Iterator() = default;
  Iterator(pointer p) : _ptr(p) {}

  reference operator*() const { return *_ptr; }
  pointer operator->() const { return _ptr; }

  Iterator &operator++() {
    _ptr++;
    return *this;
  }
  Iterator operator++(int) {
    Iterator tmp = *this;
    ++(*this);
    return tmp;
  }
  Iterator &operator+=(int i) {
    _ptr += i;
    return *this;
  }
  Iterator operator+(const difference_type other) const { return _ptr + other; }
  friend Iterator operator+(const difference_type value,
                            const Iterator &other) {
    return other + value;
  }

  Iterator &operator--() {
    _ptr--;
    return *this;
  }
  Iterator operator--(int) {
    Iterator tmp = *this;
    --(*this);
    return tmp;
  }
  Iterator &operator-=(int i) {
    _ptr -= i;
    return *this;
  }
  difference_type operator-(const Iterator &other) const {
    return _ptr - other._ptr;
  }
  Iterator operator-(const difference_type other) const { return _ptr - other; }
  friend Iterator operator-(const difference_type value,
                            const Iterator &other) {
    return other - value;
  }

  reference operator[](difference_type idx) const { return _ptr[idx]; }

#if defined(__cplusplus) && (__cplusplus == 202002L)
  auto operator<=>(const Iterator &) const = default;
#else
  bool operator==(const Iterator &other) const { return other._ptr == _ptr; }
  bool operator!=(const Iterator &other) const { return other._ptr != _ptr; }
  bool operator<=(const Iterator &other) const { return other._ptr <= _ptr; }
  bool operator<(const Iterator &other) const { return other._ptr < _ptr; }
  bool operator>=(const Iterator &other) const { return other._ptr >= _ptr; }
  bool operator>(const Iterator &other) const { return other._ptr > _ptr; }
#endif

private:
  pointer _ptr;
};
} // namespace IterGen
#endif
