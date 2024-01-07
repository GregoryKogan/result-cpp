/// @file
/// @brief result is a C++ library that provides a Result<T, E> type, which can be used to return and propagate
/// errors. It's inspired by Rust's std::Result type.
///
/// This file contains the declaration of the Result class template.
/// @author GregoryKogan
/// @version 0.0.1
/// @date 2023, December 21
/// @par License
/// This software is released under the GNU GENERAL PUBLIC LICENSE Version 3.
/// @par Contact
/// My github page: https://github.com/GregoryKogan

#ifndef RESULT_LIB
#define RESULT_LIB

#include <stdexcept>
#include <utility>

namespace res {

template <typename T, typename E> class result;
template <typename E> class result<void, E>;

/// @brief Err object represents an unsuccessful outcome and can be implicitly converted to a result.
/// @details Err object holds an error of type E. Err object can't be empty, E must not be std::monostate.
/// @tparam E Type of the error.
template <typename E> class err {
  E error_;

public:
  err() = delete;
  explicit err(E error) : error_(std::move(error)) {}

  template <typename T> operator result<T, E>() const; // NOLINT(google-explicit-constructor)
  operator result<void, E>() const;                    // NOLINT(google-explicit-constructor)
};

template <typename E> template <typename T> inline err<E>::operator result<T, E>() const {
  auto res = result<T, E>();
  res.make_unsuccessful(error_);
  return res;
}

template <typename E> inline err<E>::operator result<void, E>() const {
  auto res = result<void, E>();
  res.make_unsuccessful(error_);
  return res;
}

} // namespace res

#include <variant>

namespace res {

template <typename T, typename E> class result;
template <typename E> class result<void, E>;

/// @brief Ok object represents a successful outcome and can be implicitly converted to a result.
/// @details Ok object holds a value of type T. Empty Ok object can be created if T is std::monostate.
/// @tparam T Type of the value. Defaults to std::monostate.
template <typename T = std::monostate> class ok {
  T value_;

public:
  ok() = default;
  explicit ok(T value) : value_(std::move(value)) {}

  template <typename E> operator result<T, E>() const;    // NOLINT(google-explicit-constructor)
  template <typename E> operator result<void, E>() const; // NOLINT(google-explicit-constructor)
};

template <typename T> template <typename E> inline ok<T>::operator result<T, E>() const {
  auto res = result<T, E>();
  res.make_successful(value_);
  return res;
}

template <typename T> template <typename E> inline ok<T>::operator result<void, E>() const {
  auto res = result<void, E>();
  res.make_successful();
  return res;
}

} // namespace res

namespace res {

/// @brief `result` is a type that represents either success or failure.
///
/// result<T, E> is the type used for returning and propagating errors. It holds either a successful value of type T or
/// an error of type E.
/// @tparam T
/// @tparam E
template <typename T, typename E> class result {
  static_assert(!std::is_same_v<T, void>, "T (value type) must not be void");
  static_assert(!std::is_same_v<E, void>, "E (error type) must not be void");

  bool successful_ = false;
  T value_;
  E error_;

  result() = default;

  auto make_successful(T value) -> void {
    successful_ = true;
    value_ = std::move(value);
  }
  auto make_unsuccessful(E error) -> void {
    successful_ = false;
    error_ = std::move(error);
  }

  template <typename U> friend class ok;
  template <typename U> friend class err;

public:
  // Observers
  [[nodiscard]] auto is_ok() const -> bool { return successful_; }
  explicit operator bool() const { return is_ok(); }
  auto operator!() const -> bool { return !is_ok(); }

  [[nodiscard]] auto value() const -> const T &;
  [[nodiscard]] auto value_or(T &&default_value) const -> T;
  [[nodiscard]] auto error() const -> const E &;

  // Monadic operations
  // map method gets functor F(T x) -> U as an argument and returns result of applying this functor to the value of the
  // result object. If the result object is an error, the functor is not called and the error is propagated. But the
  // result value type is changed to U.
  template <typename F, typename U = std::invoke_result_t<F, T>> auto map(F &&functor) const -> result<U, E> {
    if (is_ok()) { return ok<U>(std::forward<F>(functor)(value_)); }
    return err<E>(error_);
  }
};

/// @brief `result` class specialization for void value type.
template <typename E> class result<void, E> {
  static_assert(!std::is_same_v<E, void>, "E (error type) must not be void");

  bool successful_ = false;
  E error_;

  result() = default;

  auto make_successful() -> void { successful_ = true; }
  auto make_unsuccessful(E error) -> void {
    successful_ = false;
    error_ = std::move(error);
  }

  template <typename U> friend class ok;
  template <typename U> friend class err;

public:
  // Observers
  [[nodiscard]] auto is_ok() const -> bool { return successful_; }
  explicit operator bool() const { return is_ok(); }
  auto operator!() const -> bool { return !is_ok(); }

  [[nodiscard]] auto error() const -> const E &;

  // Monadic operations
  // map method gets functor F() -> U as an argument and returns result of applying this functor to the value of the
  // result object. If the result object is an error, the functor is not called and the error is propagated. But the
  // result value type is changed to U.
  template <typename F, typename U = std::invoke_result_t<F>> auto map(F &&functor) const -> result<U, E> {
    if (is_ok()) { return ok<U>(std::forward<F>(functor)()); }
    return err<E>(error_);
  }
};

template <typename T, typename E> inline auto result<T, E>::value() const -> const T & {
  if (!is_ok()) { throw std::logic_error("value() called on result with error"); }
  return value_;
}

template <typename T, typename E> inline auto result<T, E>::value_or(T &&default_value) const -> T {
  if (is_ok()) { return value_; }
  return std::move(default_value);
}

template <typename T, typename E> inline auto result<T, E>::error() const -> const E & {
  if (is_ok()) { throw std::logic_error("error() called on result with value"); }
  return error_;
}

template <typename E> inline auto result<void, E>::error() const -> const E & {
  if (is_ok()) { throw std::logic_error("error() called on result with value"); }
  return error_;
}

} // namespace res

#endif // RESULT_LIB