#ifndef ITERGEN
#define ITERGEN

#include <iterator>
namespace IterGen {

template <typename T> struct Iterator {
  using iterator_concept [[maybe_unused]] = std::contiguous_iterator_tag;
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

  auto operator<=>(const Iterator &) const = default;

private:
  pointer _ptr;
};
} // namespace IterGen
#endif
