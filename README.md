# optional_view
optional_view is a C++ library for non-owning optional data, compatible with multiple allocation types (stack, heap, etc) 

Read more on ["Using optional_view to manage optional references in C++"](https://igormcoelho.medium.com/using-optional-view-to-manage-optional-references-in-c-1368abea30bb).

**In short**: this is **an immutable view to an optional type with underlying data T**, where `T` can be mutable or not, as in `T* const` (do confuse with `const T*`). 
So, `optional_view<T>` represents an immutable optional view to a mutable reference to T (like `T&`), while `optional_view<const T>` represents an immutable optional view to a const reference to T (like `const T&`).

## Rationale

What is an `optional_view<T>`: a non-owning reference to some existing type T, or empty (nullopt).
In other words, it is **immutably tied** to some reference `T&` or empty.

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

3. Mutability: `optional_view<T>` is immutably tied to some reference (or empty), 
but it can also allow modifications to the underlying object if it is non-const. 
This behavior is similar on `optional<T>`, by its nature, 
but as `optional<T&>` implementations typically rebind on practice (see discussions above),
it cannot be considered immutable.
Maybe, non-constness of `optional_view<T>` could be removed and some specific `optional_mutable_view<T>` be created...
it is debatable.
For now, one can use `optional_view<const T>` to explicitly represent immutability of referenced/underlying data.

4. Nullability: Both implementations support the notion of an optional value, as `std::nullopt`.

5. Compatibility with other types: `optional_view<T>` can be used to access many types (as many as `T*` supports), such as:
`T&`, `optional<T>`, `unique_ptr<T>`, ... Note that binding to `T*` is explicitly forbidden, to not encourage raw pointer usage.
On the other hand, `optional<T>` implies some specific memory handling mechanism, 
as it is not clear which memory model applies to `optional<T&>`. 

### Extensions

#### Reset extension
A method `reset()` is considered an extension of `optional_view`, in the same way that
`string_view` does not have a `reset()` or `clear()` method 
(better demonstrating its immutability and non-owning behavior).

However, if one may get to a situation where a `reset()` function is useful for `optional_view`,
 please let me know!

#### Lifetime extension and move constructor

By default, there is no move constructor on `optional_view`. 
This limits situations where some rvalue `int` could be directly passed into a function,
which is accepted in C++ as a `const int&` in a behavior known as "lifetime extension".
So, `optional_view` will reject this situation for three reasons: 

- (i) lifetime extension is typically a compiler feature 
- (ii) `optional_view` aims to be compatible with `string_view`, known to not need move semantics
- (iii) if it takes temporary ownership of the resource, then it needs to clear that data afterwards, which is doable... however, its copy behavior will be affected afterwards, as
this resource could be cleared in one `optional_view`, thus breaking another

So, possible extensions are: 

- (i) `optional_unique_view`, that disables copy behavior and focuses on move-only semantics (just as `unique_ptr`) - see [include/opview/optional_unique_view.hpp](include/opview/optional_unique_view.hpp)
- (ii) create `optional_shared_view`, that allows both copy and move semantics, thus storing
a `shared_ptr` to the underlying data only in cases where ownership is needed for "lifetime extension"

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
  // Check if reset() extension is available
#ifdef OPTIONAL_VIEW_EXTENSIONS
  *op_y = 90;
  op_y.reset();                          // disengage the optional
  std::cout << (bool)op_y << std::endl;  // prints 0 (FALSE)
  std::cout << *op_y << std::endl;       // BROKEN? prints 90 (?)
  std::cout << *oz << std::endl;         // BROKEN? prints 90 (?)
#endif

  return 0;
}
```

### Usage

Just copy the header located at [include/opview/optional_view.hpp](include/opview/optional_view.hpp).

Remember that C++17 is required, due to `std::optional` compatibility.

Have fun!

### Acknowledgements

Thanks Fellipe Pessanha for fruitful discussions on reset extension,
and to the C++ community for discussions on optional references.

## License

MIT LICENSE, 2023
