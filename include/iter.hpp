#ifndef LIBGETARGVPLUSPLUS_ITERATOR_H
#define LIBGETARGVPLUSPLUS_ITERATOR_H

#include <iterator>
namespace Getargv {

  /** \brief A template that turns a pointer to a C style array into a C++
   * contiguous iterator.
   *
   * The type parameter for this template is the \ref element_type that the C
   * array's pointer points to. The generated Iterator struct is a contiguous
   * iterator since C++ 20, and a random access iterator in C++ versions
   * prior to C++ 20.
   *
   * All operations are pointer math, which is a big reason why this is a
   * template, so I only have to make sure one impl works.
   */
  template <typename T>
struct Iterator {
#if defined(__cplusplus) && (__cplusplus >= 202002L)
  /** \brief A marker for the capabilities of this Iterator.
   * This is the strongest type of iterator in C++ >= 20, and we can satisfy the
   * requirements, so users can use this Iterator with as many algorithms from
   * \ref std::algorithm as possible.
   */
  using iterator_concept [[maybe_unused]] = std::contiguous_iterator_tag;
#else
  /** \brief A marker for the capabilities of this Iterator.
   * This is the strongest type of iterator in C++ < 20, and we can satisfy the
   * requirements, so users can use this Iterator with as many algorithms from
   * \ref std::algorithm as possible.
   */
  using iterator_category = std::random_access_iterator_tag;
#endif
  /** \brief This is the type that represents the distance between two
   * Iterators. The Iterators being compared must point to the same backing
   * buffer, or you get UB.
   */
  using difference_type = std::ptrdiff_t;

  /** \brief This is the type of the elements this iterator provides access to.
   * The type of the iterator's elements/values is provided as a template
   * parameter when this template is instantiated.
   */
  using element_type = T;

  /** \brief This is the type of the values this iterator provides access to.
   * The type of the iterator's elements/values is provided as a template
   * parameter when this template is instantiated.
   * \remark Clang++ doesn't seem to need this type specified, but everyone
   * online says it's needed…
   */
  using value_type = element_type;

  /** \brief The type of a pointer to an element this Iterator provides access
   * to.
   */
  using pointer = element_type*;

  /** \brief The type of a reference to an element this Iterator provides access
   * to.
   * \warning Trying to create a reference from an end Iterator or an Iterator
   * that has been incremented or decremented beyond the underlying array is UB.
   */
  using reference = element_type&;

  /** \brief The default constructor.
   * This constructor creates an empty iterator with no backing buffer. The
   * iterator this constructor creates is not usable for anything, but is
   * required to conform to the contiguous_iterator concept.
   */
  Iterator() = default;

  /** \brief A constructor that takes a pointer (C array).
   *
   * \param ptr A pointer to an array of values/elements of \ref element_type, or
   * 1 past the end of the array in the case of the end iterator.
   *
   * \warning The iterator does not own the backing buffer, so the iterator is
   * invalidated when the buffer is freed. There is no way of knowing based on
   * the iterator itself if it has been invalidated.
   *
   * \warning The type pointed to by the pointer passed to this constructor must
   * match the type parameter used when instantiating the template.
   */
  explicit Iterator(pointer ptr) : _ptr(ptr) {}

  /** \brief The indirection operator.
   *
   * This operator returns the underlying \ref element_type the iterator
   * currently points to.
   *
   * \returns A \ref reference to the element of the array that the
   * Iterator currently points to.
   *
   * \warning Trying to dereference an end iterator or an
   * iterator that has been incremented to or beyond the end iterator, or
   * decremented beyond the start iterator is UB.
   */
  auto operator*() const -> reference { return *_ptr; }

  /** \brief The pointer to member of pointer operator.
   *
   * This operator is only useful when the \ref element_type of this iterator is
   * a struct or class, and allows you to access members of the element pointed
   * to by the iterator.
   *
   * \returns the underlying pointer of this Iterator. But more practically, the
   * compiler then uses that pointer to provide access to a member of the
   * pointed to struct.
   *
   * \warning Trying to use this operator on an end iterator or an iterator that
   * has been incremented or decremented beyond the bounds of the underlying
   * array is UB.
   */
  auto operator->() const -> pointer { return _ptr; }

  /** \brief The prefix increment operator.
   *
   * This operator advances the iterator and returns itself.
   *
   * \returns itself
   */
  auto operator++() -> Iterator& {
    _ptr++;
    return *this;
  }

  /** \brief The postfix increment operator.
   *
   * This operator advances the iterator and returns an Iterator pointing to the
   * old position.
   *
   * \returns an Iterator pointing to the old position of this iterator.
   */
  auto operator++(int) -> Iterator {
    Iterator tmp = *this;
    _ptr++;
    return tmp;
  }

  /** \brief The addition assignment operator.
   *
   * Advances the Iterator by the passed in number of positions and returns
   * itself.
   *
   * \param offset the number of elements to advance the iterator by.
   *
   * \returns itself
   */
  auto operator+=(int offset) -> Iterator& {
    _ptr += offset;
    return *this;
  }

  /** \brief The addition operator.
   *
   * \param other the number of elements the returned iterator should be
   * advanced by.
   *
   * \returns an Iterator advanced by the passed in number of positions.
   */
  auto operator+(const difference_type other) const -> Iterator { return _ptr + other; }

  /** \brief The friend addition operator.
   *
   * \param value the number of elements the returned Iterator should be
   * advanced by.
   *
   * \param other the Iterator that provides the address to advance
   * from.
   *
   * \returns an iterator which is the passed in Iterator advanced by the passed
   * in number of positions.
   */
  friend auto operator+(const difference_type value,
                        const Iterator&       other) -> Iterator {
    return other + value;
  }

  /** \brief The prefix decrement operator.
   *
   * This operator recedes the iterator and returns itself.
   *
   * \returns itself
   */
  auto operator--() -> Iterator& {
    _ptr--;
    return *this;
  }

  /** \brief The postfix decrement operator.
   *
   * This operator recedes the Iterator and returns an Iterator pointing to the
   * old position.
   *
   * \returns an Iterator pointing to the old position of this iterator.
   */
  auto operator--(int) -> Iterator {
    Iterator tmp = *this;
    _ptr--;
    return tmp;
  }

  /** \brief The subtraction assignment operator.
   *
   * Recedes the Iterator by the passed in number of positions, and returns
   * itself.
   *
   * \param offset the number of elements to recede the iterator by.
   *
   * \returns itself
   */
  auto operator-=(int offset) -> Iterator& {
    _ptr -= offset;
    return *this;
  }

  /** \brief The subtraction operator (iterator overload).
   *
   * \param other the other Iterator to find the distance from.
   *
   * \returns the distance between the passed Iterator and this one.
   */
  auto operator-(const Iterator& other) const -> difference_type {
    return _ptr - other._ptr;
  }

  /** \brief The subtraction operator (offset overload).
   *
   * \param other the number of positions to recede the returned Iterator.
   *
   * \returns an Iterator receded by the passed in number of positions.
   *
   * \warning The passed in iterator must point to the same backing buffer or
   * this is UB.
   */
  auto operator-(const difference_type other) const -> Iterator { return _ptr - other; }

  /** \brief The friend subtraction operator (iterator and offset).
   *
   * \param value the number of positions to recede the returned Iterator.
   *
   * \param other the Iterator that provides the address to recede from.
   *
   * \returns an iterator which is the passed in Iterator receded by the passed
   * in number of positions.
   */
  friend auto operator-(const difference_type value,
                        const Iterator&       other) -> Iterator {
    return other - value;
  }

  /** \brief The index operator.
   *
   * This operator returns the underlying \ref element_type the passed
   * in number of positions removed from the position the iterator currently
   * points to.
   *
   * \param idx the offset of the requested element from the current position of
   * this Iterator.
   *
   * \returns a \ref reference to an element idx positions removed from this
   * Iterator's current position.
   *
   * \warning passing in an offset that would put the iterator beyond the bounds
   * of the underlying array is UB.
   */
  auto operator[](difference_type idx) const -> reference { return _ptr[idx]; }

#if defined(__cplusplus) && (__cplusplus >= 202002L)
  /** \brief The three-way comparison operator.
   *
   * \param other The iterator to compare this one to.
   *
   * \returns \ref std::strong_ordering::less if the other iterator is less than
   * this one, \ref std::strong_ordering::equal if they are equal, and
   * \ref std::strong_ordering::greater if the other iterator is greater than
   * this one.
   */
  auto operator<=>(const Iterator& other) const = default;
#else
  /** \brief The equality operator.
   */
  bool operator==(const Iterator& other) const { return other._ptr == _ptr; }
  /** \brief The not equal to operator.
   */
  bool operator!=(const Iterator& other) const { return other._ptr != _ptr; }
  /** \brief The less than or equal to operator.
   */
  bool operator<=(const Iterator& other) const { return other._ptr <= _ptr; }
  /** \brief The less than operator.
   */
  bool operator<(const Iterator& other) const { return other._ptr < _ptr; }
  /** \brief The greater than or equal to operator.
   */
  bool operator>=(const Iterator& other) const { return other._ptr >= _ptr; }
  /** \brief The greater than operator.
   */
  bool operator>(const Iterator& other) const { return other._ptr > _ptr; }
#endif

private:
  pointer _ptr;
};
} // namespace Getargv
#endif
