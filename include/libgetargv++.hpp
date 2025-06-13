#ifndef LIBGETARGVPLUSPLUS_H
#define LIBGETARGVPLUSPLUS_H

#ifdef __cplusplus

#if defined(__has_cpp_attribute) && (__cplusplus >= __has_cpp_attribute(nodiscard))
#define NODISCARD [[nodiscard]]
#else
#define NODISCARD
#endif

#if defined(__cpp_deleted_function) && (__cplusplus >= __cpp_deleted_function)
#define DELETEREASON ("Since this class manages an externally malloc'd buffer, copy is not supported; use move instead.")
#else
#define DELETEREASON
#endif

#include "iter.hpp"
#include <string>
#include <vector>

#if \
defined(__cpp_lib_string_view) && (__cplusplus >= __cpp_lib_string_view) && \
defined(__cpp_lib_type_trait_variable_templates) && (__cplusplus >= __cpp_lib_type_trait_variable_templates)

#include <string_view>
#include <type_traits>

#if defined(__cpp_concepts) && (__cplusplus >= __cpp_concepts)

template<typename T>
concept StringLikeForArgv = std::is_convertible_v<T, std::string_view> && std::is_constructible_v<T, char*, char*>;
template<typename T>
concept StringLikeForArgvArgc = std::is_convertible_v<T, std::string_view> && std::is_constructible_v<T, char*>;


template<typename T>
concept StringForArgv = std::is_convertible_v<T, std::string> && std::is_constructible_v<T, char*, char*>;
template<typename T>
concept StringForArgvArgc = std::is_convertible_v<T, std::string> && std::is_constructible_v<T, char*>;

#define PARAMETER_KEY_ARGV_TO StringLikeForArgv
#define PARAMETER_KEY_ARGVARGC_TO StringLikeForArgvArgc
#define PARAMETER_KEY_ARGV_AS StringForArgv
#define PARAMETER_KEY_ARGVARGC_AS StringForArgvArgc

#define PARAMETER_VALUE_ARGV_TO T
#define PARAMETER_VALUE_ARGVARGC_TO T
#define PARAMETER_VALUE_ARGV_AS T
#define PARAMETER_VALUE_ARGVARGC_AS T

#else

#define PARAMETER_KEY_ARGV_TO typename
#define PARAMETER_KEY_ARGVARGC_TO typename
#define PARAMETER_KEY_ARGV_AS typename
#define PARAMETER_KEY_ARGVARGC_AS typename

#define PARAMETER_VALUE_ARGV_TO std::enable_if_t<std::is_constructible_v<T, char*, char*> && std::is_convertible_v<T, std::string_view>, T>
#define PARAMETER_VALUE_ARGVARGC_TO std::enable_if_t<std::is_constructible_v<T, char*> && std::is_convertible_v<T, std::string_view>, T>
#define PARAMETER_VALUE_ARGV_AS std::enable_if_t<std::is_constructible_v<T, char*, char*> && std::is_convertible_v<T, std::string>, T>
#define PARAMETER_VALUE_ARGVARGC_AS std::enable_if_t<std::is_constructible_v<T, char*> && std::is_convertible_v<T, std::string>, T>

#endif

#else

#define PARAMETER_KEY_ARGV_TO typename
#define PARAMETER_KEY_ARGVARGC_TO typename
#define PARAMETER_KEY_ARGV_AS typename
#define PARAMETER_KEY_ARGVARGC_AS typename

#define PARAMETER_VALUE_ARGV_TO typename std::enable_if<std::is_constructible<T, char*, char*>::value && std::is_convertible<T, std::string>::value, T>::type
#define PARAMETER_VALUE_ARGVARGC_TO typename std::enable_if<std::is_constructible<T, char*>::value && std::is_convertible<T, std::string>::value, T>::type
#define PARAMETER_VALUE_ARGV_AS #PARAMETER_VALUE_ARGV_TO
#define PARAMETER_VALUE_ARGVARGC_AS #PARAMETER_VALUE_ARGVARGC_TO

#endif

/** \brief This namespace isolates the library from your code.
 */
namespace Getargv {
    namespace ffi {
#include <libgetargv.h>
    using errno_t = errno_t;
    using uint = uint;
  } // namespace ffi

  /** \brief This struct provides an iterable and printable representation of
   * the arguments of the passed in pid, formatted as specified.
   * \author Camden Narzt
   *
   * This struct is a C++ implementation of the ArgvResult struct from the C
   * libgetargv. You create this struct with the as_bytes() function. This struct
   * adds the following functionality to the C struct:
   * \li It provides an iterator over the bytes of the arguments.
   * \li You can call print on the struct directly.
   * \li It reports its size, and if it is empty.
   * \li It cleans up the backing buffer automatically using the correct free
   * function.
   *
   * This struct is well suited to byte level operations, for argument level
   * operations the ArgvArgc struct is a better fit, as it is indexed by argument.
   *
   * \remark Due to being backed by a buffer allocated by the C lib, this struct
   * cannot be copied, only moved.
   */
  struct Argv : protected ffi::ArgvResult {
    /** \brief The type of iterator provided by begin() and end()
     *
     * This iterator is a \ref std::contiguous_iterator_tag (since C++ 20) or a
     * \ref std::random_access_iterator_tag (prior to C++ 20). This is an iterator
     * over the bytes (char) of the arguments this struct represents.
     */
    using Iterator = Iterator<char>;

    /** \brief This function creates an Argv struct for the args of pid, formatted
     * as specified.
     *
     * This function is an alias for the constructor with the same arguments. It
     * exists to provide a counterpart to the as_string() function.
     *
     * \param pid the pid of the process whose arguments are requested.
     *
     * \param skip the number of leading arguemnts to skip over.
     *
     * \param nuls whether to replace delimiting ␀ bytes with spaces for human
     * consumption.
     *
     * \return The Argv struct representing the arguments of the targetted pid,
     * formatted as requested.
     *
     * \throw std::system_error If the underlying sysctl fails, an exception is
     * thrown containing the errno.
     *
     * \sa as_string()
     * \sa Argv(pid,skip,nuls)
     */
    static auto as_bytes(pid_t pid, unsigned int skip = 0, bool nuls = false) noexcept(false) -> Argv;

    /** \brief This function constructs an Argv but then converts the result into
     * a \ref std::string.
     *
     * This function creates a \ref std::string representing the args of pid,
     * formatted as specified. It calls the Argv(pid, skip, nuls) constructor, and
     * takes the same arguments and raises the same exceptions.
     *
     * \attention Note that the arguments of a process on macOS are not guaranteed
     * to be in any encoding, and therefore should be treated with caution.
     * Particularly, unless nuls is set to true, there can be internal ␀ bytes in
     * the returned string.
     *
     * \param pid the pid of the process whose arguments are requested.
     *
     * \param skip the number of leading arguemnts to skip over.
     *
     * \param nuls whether to replace delimiting ␀ bytes with spaces for human
     * consumption.
     *
     * \return A std::string representing the arguments of the targetted pid,
     * formatted as requested.
     *
     * \throw std::system_error If the underlying sysctl fails, an exception is
     * thrown containing the errno.
     *
     * \sa as_bytes()
     */
    template <PARAMETER_KEY_ARGV_AS T>
    NODISCARD static auto as_string(pid_t pid, unsigned int skip = 0, bool nuls = false) noexcept(false) -> PARAMETER_VALUE_ARGV_AS;

    /** \brief This function converts an Argv into a \ref std::string or \ref std::string_view.
     *
     * This function creates a \ref std::string or \ref std::string_view representing the args of the Argv.
     *
     * \attention Note that the arguments of a process on macOS are not guaranteed
     * to be in any encoding, and therefore should be treated with caution.
     * Particularly, unless nuls is set to true, there can be internal ␀ bytes in
     * the returned string.
     *
     * \return A std::string or std::string_view representing the arguments of the targetted pid,
     * formatted as requested.
     *
     * \sa as_string()
     */
    template <PARAMETER_KEY_ARGV_TO T>
    NODISCARD auto to_string() const noexcept(false) -> PARAMETER_VALUE_ARGV_TO;

    /**
     * Due to being backed by a buffer allocated by the C lib, this struct cannot
     * be copied, only moved.
     *
     * Rationale for non-copyability:
     * \li memory blocks returned from calls to malloc must be released by calls
     * to free from the same allocator.
     * \li this struct must clean up its backing buffer using the provided C-lib
     * function to release the memory to ensure the correct free() is called.
     * \li if a shallow copy were made of this struct, the buffer would be
     * released by each copy of the struct which is not allowed.
     * \li if a new backing buffer were allocated from C++, the call to the C
     * free() function may not match.
     * \li adding additional member variables to track if the buffer was allocated
     * from C or C++ could change the layout of the struct, making the C function
     * that frees the buffer not work.
     */
    Argv(const Argv& other) = delete DELETEREASON;

    /** \brief This is a constructor for the Argv struct representing the args of
     * pid, formatted as specified.
     *
     * This constructor wraps the C api for creating an ffi::ArgvResult and
     * handles failure by throwing an exception.
     *
     * \param pid the pid of the process whose arguments are requested.
     *
     * \param skip the number of leading arguemnts to skip over.
     *
     * \param nuls whether to replace delimiting ␀ bytes with spaces for human
     * consumption.
     *
     * \throw std::system_error If the underlying sysctl fails, an exception is
     * thrown containing the errno.
     *
     * \sa as_bytes
     */
    explicit Argv(pid_t pid, unsigned int skip = 0, bool nuls = false) noexcept(false);

    Argv(Argv&& other) = default; ///< Default move constructor

    /** \brief Constructor to convert C struct into C++ struct
     *
     * \param ffiResult the ffi::ArgvResult to convert into an Argv.
     *
     * \warning Do not free the C struct's buffer after calling this constructor,
     * it is adopted by this struct, and freed when this struct is destructed.
     */
    explicit Argv(ffi::ArgvResult&& ffiResult);

    /** \brief This is the destructor for this struct.
     *
     * This destructor calls the C api to clean up the backing buffer using the
     * correct free() function. Due to the need for malloc/free to match, this
     * destructor is not safe for buffers allocated from C++.
     */
    ~Argv();

    /** \brief index operator to access a byte (char) at supplied offset (0
     * indexed)
     *
     * \param index the offset into the byte array of the requested byte.
     *
     * \returns the byte (char) at the passed offset.
     */
    auto operator[](ptrdiff_t index) const -> char&;

    auto operator=(Argv&& other) -> Argv& = default;
    auto operator=(Argv& other) -> Argv& = delete DELETEREASON;

    /** \brief returns the number of bytes in the arguments this struct
     * represents.
     *
     * \returns the number of bytes in the arguments this struct represents.
     */
    NODISCARD auto size() const -> ptrdiff_t;

    /** \brief returns true if the process whose arguments this struct
     * represents either had no arguments or they were all skipped.
     *
     * \returns true if the process whose arguments this struct represents either
     * had no arguments or they were all skipped.
     */
    NODISCARD auto empty() const -> bool;

    /** \brief returns a begin iterator.
     *
     * \returns an Iterator pointing to the beginning of the bytes of the
     * arguments this struct represents.
     *
     * \sa Iterator
     */
    NODISCARD auto begin() const -> Iterator;

    /** \brief returns an end iterator.
     *
     * \returns an Iterator pointing to just after the end of the arguments this
     * struct represents.
     *
     * \sa Iterator
     */
    NODISCARD auto end() const -> Iterator;

    /** \brief Prints the bytes of the arguments this struct represents to stdout.
     *
     * This method prints the bytes to stdout including the delimiting bytes
     * (whether ␀ bytes or spaces) from the first argument not skipped over, to
     * the last.
     *
     * \throw std::system_error If printing fails, an exception is
     * thrown containing the errno.
     */
    void print();
  };

  /** \brief This struct provides an iterable representation of the arguments of
   * the passed in pid.
   *
   * \author Camden Narzt
   *
   * This struct is a C++ implementation of the ArgvArgcResult struct from the C
   * libgetargv. You create this struct with the as_array() function. This struct
   * adds the following functionality to the C struct:
   * \li It provides an iterator over the arguments.
   * \li It reports its size, and if it is empty.
   * \li It cleans up the backing buffers automatically using the correct free
   * function.
   *
   * This struct is best suited to inspection and/or manipulation of the
   * arguments. If you only want to print the arguments, then the Argv struct is a
   * better choice.
   *
   * \remark Due to being backed by buffers allocated by the C lib, this struct
   * cannot be copied, only moved.
   */
  struct ArgvArgc : protected ffi::ArgvArgcResult {
    /** \brief The type of iterator provided by begin() and end()
     *
     * This iterator is a \ref std::contiguous_iterator_tag (since C++ 20) or a
     * \ref std::random_access_iterator_tag (prior to C++ 20). This is an iterator
     * over the C strings (char*) representating the arguments this struct
     * represents.
     */
    using Iterator = Iterator<char*>;

    /** \brief This function creates an ArgvArgc struct for the args of pid.
     *
     * This function is an alias for the constructor with the same argument. It
     * exists to provide a counterpart to the as_array() function.
     *
     * \param pid the pid of the process whose arguments are requested.
     *
     * \return The ArgvArgc struct representing the arguments of the targetted
     * pid.
     *
     * \throw std::system_error If the underlying sysctl fails, an exception is
     * thrown containing the errno.
     *
     * \sa as_array()
     * \sa ArgvArgc(pid)
     */
    static auto as_array(pid_t pid) noexcept(false) -> ArgvArgc;

    /** \brief This function constructs an ArgvArgc but then converts the result
     * into a \ref std::vector<std::string>.
     *
     * This function creates a \ref std::vector containing \ref std::string
     * representations of the args of pid. It calls the ArgvArgc(pid) constructor,
     * and takes the same argument and raises the same exceptions.
     *
     * \attention Note that the arguments of a process on macOS are not guaranteed
     * to be in any encoding, and therefore should be treated with caution.
     *
     * \param pid the pid of the process whose arguments are requested.
     *
     * \return A std::vector<std::string> representing the arguments of the
     * targetted pid.
     *
     * \throw std::system_error If the underlying sysctl fails, an
     * exception is thrown containing the errno.
     *
     * \sa as_array()
     */
    template <PARAMETER_KEY_ARGVARGC_AS T>
    NODISCARD static auto as_vector(pid_t pid) noexcept(false) -> std::vector<PARAMETER_VALUE_ARGVARGC_AS>;

    /** \brief This function converts an ArgvArgc into a \ref
     * std::vector<std::string> or std::vector<std::string_view>.
     *
     * This function creates a \ref std::vector containing \ref std::string or std::string_view
     * representations of the args represented by the ArgvArgc.
     *
     * \attention Note that the arguments of a process on macOS are not guaranteed
     * to be in any encoding, and therefore should be treated with caution.
     *
     * \return A std::vector<std::string> or std::vector<std::string_view> representing
     * the arguments of the ArgvArgc.
     *
     * \sa as_string_array()
     */
    template <PARAMETER_KEY_ARGVARGC_TO T>
    NODISCARD auto to_vector() const noexcept(false) -> std::vector<PARAMETER_VALUE_ARGVARGC_TO>;

    /**
     * Due to being backed by buffers allocated by the C lib, this struct cannot
     * be copied, only moved.
     *
     * Rationale for non-copyability:
     * \li memory blocks returned from calls to malloc must be released by calls
     * to free from the same allocator.
     * \li this struct must clean up its backing buffers using the provided C-lib
     * function to release the memory to ensure the correct free() is called.
     * \li if a shallow copy were made of this struct, the buffers would be
     * released by each copy of the struct which is not allowed.
     * \li if a new backing buffer were allocated from C++, the call to the C
     * free() function may not match.
     * \li adding additional member variables to track if the buffers were
     * allocated from C or C++ could change the layout of the struct, making the C
     * function that frees the buffers not work.
     */
    ArgvArgc(const ArgvArgc& other) = delete DELETEREASON;

    /** \brief This is a constructor for the ArgvArgc struct representing the args
     * of pid.
     *
     * This constructor wraps the C api for creating an ffi::ArgvArgcResult and
     * handles failure by throwing an exception.
     *
     * \param pid the pid of the process whose arguments are requested.
     *
     * \throw std::system_error If the underlying sysctl fails, an exception is
     * thrown containing the errno.
     *
     * \sa as_array
     */
    explicit ArgvArgc(pid_t pid) noexcept(false);

    ArgvArgc(ArgvArgc&& other) = default; ///< Default move constructor

    /** \brief Constructor to convert C struct into C++ struct
     *
     * \param ffiResult the ffi::ArgvArgcResult from which to make an ArgvArgc.
     *
     * \warning Do not free the C struct's buffers after calling this constructor,
     * they are adopted by this struct, and freed when this struct is destructed.
     */
    explicit ArgvArgc(ffi::ArgvArgcResult&& ffiResult);

    /** \brief This is the destructor for this struct.
     *
     * This destructor calls the C api to clean up the backing buffers using the
     * correct free() function. Due to the need for malloc/free to match, this
     * destructor is not safe for buffers allocated from C++.
     */
    ~ArgvArgc();

    /** \brief index operator to access a C string (char*) at supplied offset (0
     * indexed)
     *
     * \param index the offset into the underlying array for the desired argument.
     *
     * \returns a C string at the specified offset from the arguments this struct
     * represents.
     *
     * /remark C strings (char*) auto convert to \ref std::string or \ref
     * std::string_view on assignment and when passed to functions, so there's
     * no need to return a \ref std::string or \ref std::string_view here.
     */
    auto operator[](ptrdiff_t index) const -> char*&;

    auto operator=(ArgvArgc&& other) -> ArgvArgc& = default;
    auto operator=(ArgvArgc& other) -> ArgvArgc& = delete DELETEREASON;

    /** \brief returns the number of arguments this struct represents
     *
     * \returns the number of arguments this struct represents
     */
    NODISCARD auto size() const -> ptrdiff_t;

    /** \brief returns true if the targetted process had no arguments
     *
     * \returns true if the targetted process had no arguments
     */
    NODISCARD auto empty() const -> bool;

    /** \brief returns a begin iterator.
     *
     * \returns an iterator pointing to the first argument represented by this
     * struct.
     *
     * \sa Iterator
     */
    NODISCARD auto begin() const -> Iterator;

    /** \brief returns an end iterator.
     *
     * \returns an iterator pointing to just after the last argument represented
     * by this struct.
     *
     * \sa Iterator
     */
    NODISCARD auto end() const -> Iterator;
  };

} // namespace Getargv

#endif // __cplusplus
#endif // LIBGETARGVPLUSPLUS_H
