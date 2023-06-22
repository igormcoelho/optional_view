// SPDX-License-Identifier: MIT
// Copyright (C) 2023 - optional_view
// https://github.com/igormcoelho/optional_view

// demo for optional view

#include <iostream>
#include <opview/optional_view.hpp>

using opview::optional_view;

void f(optional_view<int> maybe_int) {
  if (maybe_int)
    std::cout << *maybe_int << std::endl;
  else
    std::cout << "empty" << std::endl;
}

int main() {
  int x = 10;
  optional_view<int> ox{x};
  f(ox);            // prints 10
  f(std::nullopt);  // prints "empty"
  // f(10);         // ERROR: no move semantics (non-ownership)
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
  *op_y = 90;
  op_y.reset();                          // disengage the optional
  std::cout << (bool)op_y << std::endl;  // prints 0 (FALSE)
  std::cout << *op_y << std::endl;       // BROKEN? prints 90 (?)
  std::cout << *oz << std::endl;         // BROKEN? prints 90 (?)

  return 0;
}
