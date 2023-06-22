// SPDX-License-Identifier: MIT
// Copyright (C) 2023 - optional_view
// https://github.com/igormcoelho/optional_view

#ifndef OPVIEW_OPTIONAL_UNIQUE_VIEW_HPP_
#define OPVIEW_OPTIONAL_UNIQUE_VIEW_HPP_

// #define OPTIONAL_VIEW_EXTENSIONS 1

// Optional Unique View:
// This is an alternative version to optional_view,
// where Lifetime Extension (such as in const int&)
// can be treated accordingly.
// In this case, we provide Move Semantics, but we
// block Copy behavior, to maintain "uniqueness"
// We treat as if resource is non-owned by optional_unique_view,
// as in optional_view. But on practice, sometimes it
// may own the resource temporarily, to keep it alive as
// in lifetime extension.

#include <memory>    // for std::unique_ptr
#include <optional>  // for std::nullopt
#include <utility>   // for std::move

namespace opview {
//
template <typename T>
class optional_unique_view {
  using value_type = T;

 private:
  std::unique_ptr<T> value;
  bool is_owner{false};  // default is 'false' here

 public:
  optional_unique_view() : value{nullptr} {}

  // do not accept pointer here
  // explicit optional_unique_view(T* _value) : value{_value} {}

  // this is unsafe: but the risk is yours! (explicit or implicit)
  // NOLINTNEXTLINE
  optional_unique_view(T& _value) : value{&_value}, is_owner{false} {}

  // support rvalue for lifetime extension
  // NOLINTNEXTLINE
  optional_unique_view(T&& _value)
      : value{new T{std::move(_value)}}, is_owner{true} {}

  // allow nullopt (explicit or implicit)
  // NOLINTNEXTLINE
  optional_unique_view(std::nullopt_t data) : value{nullptr}, is_owner{false} {}

  // disallow nullptr
  // NOLINTNEXTLINE
  optional_unique_view(std::nullptr_t data) = delete;

  // allow optional<T> for compatibility (explicit or implicit)
  // NOLINTNEXTLINE
  optional_unique_view(std::optional<T>& op_data)
      : value{op_data ? &(*op_data) : nullptr}, is_owner{false} {}

  template <class X, typename = typename std::enable_if<
                         std::is_convertible<X*, T*>::value ||
                         std::is_same<X, T>::value>::type>
  optional_unique_view(std::optional<X>& op_data)
      : value{op_data ? &(*op_data) : nullptr}, is_owner{false} {}

  // ===============================================

  // disallow copy constructor
  optional_unique_view(const optional_unique_view<T>& other) = delete;

  // enable move constructor
  optional_unique_view(optional_unique_view<T>&& other) noexcept
      : value{std::move(other.value)}, is_owner{other.is_owner} {}

  ~optional_unique_view() {
    if (!is_owner) value.release();  // prevent double-free
    value = nullptr;
  }

  // MUST delete all operator=
  // This is coherent to *_view behavior, and also prevent misleading issues
  // with possible rebind or not rebind... this is not needed on a view.
  optional_unique_view<T>& operator=(const optional_unique_view<T>&) = delete;

  optional_unique_view<T>& operator=(optional_unique_view<T>&&) = delete;

  // return raw pointer
  T* operator->() { return value.get(); }

  // return raw pointer
  const T* operator->() const { return value.get(); }

  // return dereferenced shared object
  T& operator*() { return *value; }

  // return dereferenced shared object
  const T& operator*() const { return *value; }

  // return dereferenced shared object
  T& get() { return *value; }

  // return dereferenced shared object
  const T& get() const { return *value; }

  // return dereferenced shared object
  operator T&() { return *value; }

  bool empty() const { return !(value); }

  // has some view?
  operator bool() { return (bool)value; }

#ifdef OPTIONAL_VIEW_EXTENSIONS
  void reset() noexcept {
    if (!is_owner) value.release();  // prevent double-free
    value = nullptr;
  }
#endif
};

}  // namespace opview

#endif  // OPVIEW_OPTIONAL_UNIQUE_VIEW_HPP_
