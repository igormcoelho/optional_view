# optional_view
optional_view is a C++ library for non-owning optional data, compatible with multiple allocation types (stack, heap, etc) 

## Rationale

What is an `optional_view<T>`: a non-owning reference to some existing type T, or empty (nullopt)

This is useful in situations where you may pass or not some variable,
typically resolved with `T*`. 
Also useful on situations where `std::optional<T&>` could solve the same problem,
but optional references typically lead to crazy discussions... so, let's
focus on optional_view, and give it proper semantics.

This is achieved by disabling operator=, as in `std::string_view` so as
move semantics, due to explicit non-ownership behavior.

This **can increase efficiency** in code, 
as it **prevents the costs** of packing into std::optional when data is
already loaded into any memory model, such as stack, heap, any smart
pointer or even optional type (all abstracted away as T*)

### Short comparison with `optional<T&>`

1. Ownership and lifetime management: `optional_view<T>` implementation, the container does not own the referenced object. It is a non-owning view that relies on the object's lifetime being managed elsewhere. On the other hand, `optional<T>` takes ownership of the referenced object and ensures its lifetime by managing it as part of the container.
It is unclear (at least to me), if `optional<T&>` takes ownership on the data.

2. Binding and rebinding: `optional_view<T>` avoids the rebind issue associated with references by accepting the object by value and providing a view-like interface. It eliminates the need for rebinding references when passing objects to the container.
On `optional<T&>` this behavior is not clear ([see discussions](https://herbsutter.com/2020/02/23/references-simply/))

3. Mutability: `optional_view<T>` implementation can be designed to allow modifications to the underlying object if it is non-const. This behavior is similar on `optional<T>`, by its nature.
Maybe, non-constness could be removed and some specific `optional_mutable_view<T>` be created...
it is debatable.
For now, `optional_view<const T>` explicitly represents immutability.

4. Nullability: Both implementations support the notion of an optional value, as `std::nullopt`.


### Demo

See the [demo/main.cpp](demo/main.cpp) or snippet below:

```{.cpp}
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
  *op_y = 90;
  op_y.reset();                          // disengage the optional
  std::cout << (bool)op_y << std::endl;  // prints 0 (FALSE)
  std::cout << *op_y << std::endl;       // BROKEN? prints 90 (?)
  std::cout << *oz << std::endl;         // BROKEN? prints 90 (?)

  return 0;
}
```

### Usage

Just copy the header located at [include/opview/optional_view.hpp](include/opview/optional_view.hpp).

Remember that C++17 is required, due to `std::optional` compatibility.

Have fun!

## License

MIT LICENSE, 2023
