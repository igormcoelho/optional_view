// SPDX-License-Identifier: MIT
// Copyright (C) 2023 - optional_view
// https://github.com/igormcoelho/optional_view

// demo for optional view

// #define OPTIONAL_VIEW_EXTENSIONS

#include <iostream>
#include <memory>
#include <opview/optional_unique_view.hpp>
#include <opview/optional_view.hpp>

using opview::const_optional_view;
using opview::optional_unique_view;
using opview::optional_view;

void f(optional_view<int> maybe_int) {
  if (maybe_int)
    std::cout << *maybe_int << std::endl;
  else
    std::cout << "empty" << std::endl;
}

void g(optional_unique_view<int> maybe_int) {
  if (maybe_int)
    std::cout << *maybe_int << std::endl;
  else
    std::cout << "empty" << std::endl;
}

int main() {
  int x = 10;
  f(x);  // prints 10
  //
  optional_view<int> ox{x};
  f(ox);            // prints 10
  f(std::nullopt);  // prints "empty"
  // f(10);         // ERROR: no move semantics (non-ownership)
  auto z = std::make_unique<int>(5);
  f(*z);  // OK: prints 5
  //
  std::optional<int> op_y{20};  // OK for std::optional...
  f(op_y);                      // compatible: prints 20
  //
  x = 40;    // changes x from 10 to 40
  f(ox);     // prints 40 (view behavior from x...)
  *ox = 50;  // allows mutable data change for int
  f(ox);     // prints 50
  //
  std::cout << x << std::endl;  // prints 50
  //
  std::cout << *op_y << std::endl;  // prints 20
  optional_view<const int> oz{op_y};
  // f(oz);          // ERROR: cannot const_cast from const int to int
  std::cout << *oz << std::endl;  // prints 20
  // *oz = 30;       // disallows data change for const int
  //
  *op_y = 25;                     // remote change on std::optional
  std::cout << *oz << std::endl;  // prints 25
  //
  // optional_view<int> ow{oz};  // ERROR: ‘const int’ to ‘int&’
  //
  // Check if reset() extension is available
#ifdef OPTIONAL_VIEW_EXTENSIONS
  *op_y = 90;
  op_y.reset();                          // disengage the optional
  std::cout << (bool)op_y << std::endl;  // prints 0 (FALSE)
  std::cout << *op_y << std::endl;       // BROKEN? prints 90 (?)
  std::cout << *oz << std::endl;         // BROKEN? prints 90 (?)
#endif
  std::cout << "BEGIN UNIQUE PART" << std::endl;
  int x2 = 10;
  g(x2);  // prints 10
  //
  optional_unique_view<int> ox2{x2};
  std::cout << (bool)ox2 << std::endl;  // OK: true
  // g(ox2);                            // ERROR: no copy
  g(std::move(ox2));                    // OK: prints 10
  std::cout << (bool)ox2 << std::endl;  // OK: false
  g(std::nullopt);                      // prints "empty"
  g(10);                                // OK: prints 10
  return 0;
}
