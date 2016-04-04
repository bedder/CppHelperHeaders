////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Matthew Bedder (bedder@gmail.com)
//
// This code has been released under the MIT license. See the LICENSE file in
// the project root for more information
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>

namespace optional {

////////////////////////////////////////////////////////////////////////////////
// EmptyParameterList
//   A empty utility class - This allows us to differentiate between the CTOR
//   of Optional that should default-construct T from the CTOR of Optional that
//   shouldn't construct a T object.
////////////////////////////////////////////////////////////////////////////////
class ConstructFromParameterList {};

////////////////////////////////////////////////////////////////////////////////
// Optional<T>
//   Defines a class that potentially owns an instance of T. This should not be
//   used directly - use Copyable or NonCopyable depending on whether you want
//   copy CTORs to be applied to the managed data item.
//
//   Note that any instances of T are managed using smart pointers, and
//   therefore live on the heap. It might be worth trying to avoid using
//   Copyable/NonCopyable to manage large data structures
////////////////////////////////////////////////////////////////////////////////
template <typename T>
class Optional {
public:
  // Basic operators
  operator bool() const { return static_cast<bool>(t_); }
  T* operator->()       { return t_.get(); }
  T& operator*()        { return *(t_.get()); }
  void release()        { t_.release(); }

protected:
  // CTOR - Create a Optional<T> constructing T from an argument list
  template <typename... Args>
  explicit Optional(const ConstructFromParameterList&, Args... args)
  : t_(std::make_unique<T>(args...)) {}

  // Default CTOR - Create a Optional<T> that doesn't own a T
  explicit Optional()
  : t_() {}

  // Move CTOR
  explicit Optional(std::unique_ptr<T> t)
  : t_(std::move(t)) {}

  // The managed resource
  std::unique_ptr<T> t_;
};

////////////////////////////////////////////////////////////////////////////////
// Copyable<T>
//   Defines a class that potentially owns an instance of T. Copies of Copyable
//   objects makes copies of any managed T instances, so this class should be
//   used sparingly.
////////////////////////////////////////////////////////////////////////////////
template <typename T>
class Copyable : public Optional<T> {
public:
  template <typename U, typename... Args>
  friend Copyable<U> make_copyable(Args... args);

  // Default CTOR - Create a Copyable<T> that doesn't own a T
  Copyable()
  : Optional<T>() {}

  // Copy CTORS -  These create copies of the managed resource (if any)
  Copyable(const Copyable& other)
  : Optional<T>(std::make_unique<T>(*(other.t_))) {}

  Copyable<T>& operator=(const Copyable<T>& other) {
    Optional<T>::t_ = std::make_unique<T>(*(other.t_));
    return *this;
  }

  // Move CTORs - These work as expected
  Copyable(Copyable&& other) = default;
  Copyable<T>& operator=(Copyable<T>&& other) = default;

protected:
  // CTOR of Optional constructing a T from an argument list
  template <typename... Args>
  explicit Copyable(const ConstructFromParameterList& flag, Args... args)
  : Optional<T>(flag, args...) {}
};

////////////////////////////////////////////////////////////////////////////////
// makeCopyable<T>(Args... args)
//   A helper function for creating Copyable<T> instances
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename... Args>
Copyable<T> make_copyable(Args... args) {
  return Copyable<T>::Copyable(ConstructFromParameterList(), args...);
}

////////////////////////////////////////////////////////////////////////////////
// NonCopyable<T>
//   Defines a class that potentially owns an instance of T. Copies of
//   NonCopyable objects do not copy the underlying data, and instead become
//   invalid. This could be useful, for example, to allow a class containing
//   non-copyable objects to be made partially copyable
//
//   (e.g. an object containing std::ofstream instances for logging can be
//   changed to instead use NonCopyable<std::ofstream> : this means that the
//   object can be copied, with copies not trying to copy the std::ofstream
//   objects)
////////////////////////////////////////////////////////////////////////////////

template <typename T>
class NonCopyable : public Optional<T> {
public:
  template <typename U, typename... Args>
  friend NonCopyable<U> make_noncopyable(Args... args);

  // Default CTOR - Create a NonCopyable<T> that doesn't own a T
  NonCopyable()
  : Optional<T>() {}

  // Copy CTORS -  These do NOT copy the managed resource, and invalidates any
  // locally-stored copies
  NonCopyable(const NonCopyable& other)
  : Optional<T>() {}

  NonCopyable<T>& operator=(const NonCopyable<T>& o) {
    Optional<T>::t_.release();
    return *this;
  }

  // Move CTORs - These work as expected
  NonCopyable(NonCopyable&& other) = default;
  NonCopyable<T>& operator=(NonCopyable<T>&& other) = default;

protected:
  // CTOR of Optional constructing a T from an argument list
  template <typename... Args>
  explicit NonCopyable(const ConstructFromParameterList& flag, Args... args)
  : Optional<T>(flag, args...) {}
};

template <typename T, typename... Args>
NonCopyable<T> make_noncopyable(Args... args) {
  return NonCopyable<T>::NonCopyable(ConstructFromParameterList(), args...);
}

}  // namespace optional
