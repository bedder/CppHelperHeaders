////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Matthew Bedder (bedder@gmail.com)
//
// This code has been released under the MIT license. See the LICENSE file in
// the project root for more information
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Optional.h defines two classes that can be used:
//    class Copyable<T>
//    class NonCopyable<T>
// Also defined are two helper functions:
//    Copyable<T> make_copyable<T>(Args...)
//    NonCopyable<T> make_noncopyable<T>(Args...)
//
// These are designed as simple wrappers for optional objects, and act
// differently when their copy CTORs are called:
//    Copyable<T> clones the optional object (if it exists)
//    NonCopyable<T> does not clone the optional object (if it exists), so the
//       copy of the NonCopyable<T> object is always free of any responsibility
//
// This can be useful for wrapping non-copyable members of a class to allow the
// class to be copied.
////////////////////////////////////////////////////////////////////////////////

#include <exception>       // Required for std::logic_error
#include <iostream>        // Required for std::cout, std::cerr
#include <string>          // Required for std::string
#include "Optional.h"      // Required for optional::Copyable, optional::NonCopyable
using namespace helper::optional;  // This is bad practice, but fine this time.

struct Copy { // A struct with a working copy CTOR
  Copy()            { std::cout << "called default CTOR\n"; }
  Copy(const Copy&) { std::cout << "called copy    CTOR\n"; }
};

struct NonCopy {  // A struct with a throwing copy CTOR
  NonCopy()               { std::cout << "called default CTOR\n"; }
  NonCopy(const NonCopy&) { throw std::logic_error("called copy CTOR"); }
};

// A little helper function: just checks if an optional::Optional<T> object is
// currently managing a T resource.
template <typename T> std::string valid(const Optional<T>& opt) {
  return opt ? "valid" : "invalid";
}

////////////////////////////////////////////////////////////////////////////////
// Example entry point. Comments at the ends of each line shows what we're
// expecting in the standard output stream (std::cout)
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
  // You can use optional::Copyable for objects with copy CTORs...
  {
    std::cout << "=========================================\n"
                 "Attempting to copy a Copyable instance...\n"
                 "=========================================\n";
    auto c_original = make_copyable<Copy>();                      // > called default CTOR
    auto c_copy    = c_original;                                  // > called copy    CTOR
    std::cout << "Original is " << valid(c_original) << "\n";   // > c_original is valid
    std::cout << "Copy     is " << valid(c_copy) << "\n";       // > c_copy     is valid
  }
  std::cout << "\n";
  {
    std::cout << "=========================================\n"
                 "Attempting to move a Copyable instance...\n"
                 "=========================================\n";
    auto c_original = make_copyable<Copy>();                      // > called default CTOR
    auto c_move = std::move(c_original);                          //
    std::cout << "Original is " << valid(c_original) << "\n";   // > c_original is invalid
    std::cout << "Move     is " << valid(c_move) << "\n";       // > c_move     is valid
  }

  std::cout << "\n";

  // You can use optional::NonCopyable for objects with or without copy CTORs...
  {
    std::cout << "============================================\n"
                 "Attempting to copy a NonCopyable instance...\n"
                 "============================================\n";
    auto nc_original = make_noncopyable<NonCopy>();               // > called default CTOR
    auto nc_copy    = nc_original;                                //
    std::cout << "Original is " << valid(nc_original) << "\n"; // > nc_original is valid
    std::cout << "Copy     is " << valid(nc_copy) << "\n";     // > nc_copy     is invalid
  }
  std::cout << "\n";
  {
    std::cout << "============================================\n"
                 "Attempting to move a NonCopyable instance...\n"
                 "============================================\n";
    auto nc_original = make_noncopyable<NonCopy>();               // > called default CTOR
    auto nc_move    = std::move(nc_original);                     //
    std::cout << "Original is " << valid(nc_original) << "\n"; // > nc_original is invalid
    std::cout << "Move     is " << valid(nc_move) << "\n";     // > nc_move     is valid
  }

  std::cout << "\n";

  // If you use optional::Copyable for objects without copy CTORs you're going
  // to have a bad time!
  {
    std::cout << "=================================================================\n"
                 "Attempting to use Copyable for a instance with a bad copy CTOR...\n"
                 "=================================================================\n";
    auto c_original = make_copyable<NonCopy>();  // > called NonCopy default CTOR
    try {
      // This will try to make a copy of the NonCopy resource, so throw an exception
      auto c_copy = c_original;
    } catch (const std::logic_error& e) {
      std::cerr << "Error when using the optional::Copyable<T> copy CTOR: \""
                << e.what()
                << "\".\n";
    }
  }

  // And we're done!
  return 0;
}
