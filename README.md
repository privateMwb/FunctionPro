# FunctionPro

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![Status](https://img.shields.io/badge/status-learning%20project-green)](https://github.com/privateMwb/FunctionPro)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**FunctionPro** is a from-scratch C++23 callable wrapper library built as a deep dive into type erasure, small buffer optimization, and performance benchmarking against the standard library.

---

## Table of Contents

- [Overview](#overview)
- [Motivation](#motivation)
- [Features](#features)
- [Quick Start](#quick-start)
- [Core API](#core-api)
- [Design Overview](#design-overview)
- [Complexity](#complexity)
- [Benchmarks](#benchmarks)
- [Project Structure](#project-structure)
- [Building from Source](#building-from-source)
- [Known Limitations](#known-limitations)
- [License](#license)

---

## Overview

FunctionPro provides three callable wrapper types implemented from scratch:

- `Function<R(Args...)>` — a copyable, owning callable wrapper (similar to `std::function`)
- `MoveOnlyFunction<R(Args...)>` — a move-only, owning callable wrapper (similar to `std::move_only_function`)
- `FunctionRef<R(Args...)>` — a non-owning, non-allocating callable view (similar to `std::function_ref`)

All three are built on a shared type-erasure foundation using a custom `VTable`, `VTableFactory`, and `CallableStorage` layer with Small Buffer Optimization (SBO).

---

## Motivation

This project was built to understand:

- Type erasure and vtable-based dispatch without virtual functions
- Small Buffer Optimization to avoid heap allocation for small callables
- The difference between owning and non-owning callable abstractions
- Correct copy, move, and destruction semantics for type-erased storage
- Performance characteristics compared to `std::function`
- Portability across GCC, Clang, and MSVC

---

## Features

| Feature | Description |
|---|---|
| Small Buffer Optimization | 32-byte inline buffer — most lambdas and functors require no heap allocation |
| Transparent heap fallback | Callables exceeding SBO capacity allocate on the heap automatically |
| `Function` | Copyable, owning wrapper — requires copy-constructible callables |
| `MoveOnlyFunction` | Move-only, owning wrapper — accepts move-only callables (e.g. `unique_ptr` captures) |
| `FunctionRef` | Non-owning, non-allocating view — 16-byte object size |
| Compile-time lvalue enforcement | `FunctionRef`'s constructor accepts only lvalues; rvalue binding is rejected at compile time to prevent dangling pointers |
| Shared vtable per type | `static constexpr` vtable shared across instances of the same callable type — no per-instance allocation |
| `swap` support | Member and free `swap` for all three types |
| State management | `reset()` and `operator bool` for all three types |
| Null comparison | `operator==` / `operator!=` against `nullptr` |
| Cross-compiler constraints | Concepts-based constraints with `std::enable_if_t` fallback for Clang compatibility |
| Verified portability | Tested on GCC, Clang, and MSVC |

---

## Quick Start

### Function

```cpp
#include <FunctionPro/Function.h>

using namespace FunctionPro;

int add(int a, int b) { return a + b; }

int main() {
    Function<int(int, int)> f(add);
    f(3, 4);   // 7

    int bias = 10;
    Function<int(int)> g([bias](int x) { return x + bias; });
    g(5);      // 15

    Function<int(int, int)> h(f);   // copy
    Function<int(int, int)> k(std::move(f));   // move
}
```

### MoveOnlyFunction

```cpp
#include <FunctionPro/MoveOnlyFunction.h>

#include <memory>

using namespace FunctionPro;

int main() {
    auto ptr = std::make_unique<int>(42);

    MoveOnlyFunction<int()> f([p = std::move(ptr)]() { return *p; });
    f();   // 42

    MoveOnlyFunction<int()> g(std::move(f));   // move
    g();   // 42
}
```

### FunctionRef

```cpp
#include <FunctionPro/FunctionRef.h>

using namespace FunctionPro;

int add(int a, int b) { return a + b; }

int main() {
    auto lam = [](int a, int b) { return a + b; };

    FunctionRef<int(int, int)> ref(lam);    // bind to lambda
    ref(3, 4);   // 7

    FunctionRef<int(int, int)> ref2(add);   // bind to free function
    ref2(3, 4);  // 7
}
```

### Swap

```cpp
Function<int(int, int)> a(add);
Function<int(int, int)> b([](int x, int y) { return x * y; });
a.swap(b);
swap(a, b);   // free swap
```

---

## Core API

### Function

```cpp
Function()                noexcept;
Function(std::nullptr_t)  noexcept;

template<typename T>
Function(T&& callable);          // requires copy-constructible T

Function(const Function&);
Function& operator=(const Function&);
Function(Function&&)              noexcept;
Function& operator=(Function&&)   noexcept;

~Function();

R operator()(Args... args) const;

[[nodiscard]] explicit operator bool() const noexcept;
void reset()                             noexcept;
void swap(Function&)                     noexcept;

[[nodiscard]] bool operator==(std::nullptr_t) const noexcept;
[[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
```

### MoveOnlyFunction

```cpp
MoveOnlyFunction()                noexcept;
MoveOnlyFunction(std::nullptr_t)  noexcept;

template<typename T>
MoveOnlyFunction(T&& callable);   // accepts move-only callables

MoveOnlyFunction(const MoveOnlyFunction&)            = delete;
MoveOnlyFunction& operator=(const MoveOnlyFunction&) = delete;
MoveOnlyFunction(MoveOnlyFunction&&)                  noexcept;
MoveOnlyFunction& operator=(MoveOnlyFunction&&)       noexcept;

~MoveOnlyFunction();

R operator()(Args... args);       // non-const

[[nodiscard]] explicit operator bool() const noexcept;
void reset()                             noexcept;
void swap(MoveOnlyFunction&)             noexcept;

[[nodiscard]] bool operator==(std::nullptr_t) const noexcept;
[[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
```

### FunctionRef

```cpp
FunctionRef()               noexcept;

template<typename T>
FunctionRef(T& callable)    noexcept;   // lvalue only — no rvalue binding

FunctionRef(const FunctionRef&)            = default;
FunctionRef& operator=(const FunctionRef&) = default;
FunctionRef(FunctionRef&&)                  noexcept = default;
FunctionRef& operator=(FunctionRef&&)       noexcept = default;

R operator()(Args... args) const;

[[nodiscard]] explicit operator bool() const noexcept;

[[nodiscard]] bool operator==(std::nullptr_t) const noexcept;
[[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
```

---

## Design Overview

### CallableStorage

`CallableStorage` is the shared storage layer used by `Function` and `MoveOnlyFunction`. It holds either an inline SBO buffer or a heap pointer, and exposes three distinct accessors:

```
+--------------------------+
|  buffer[32]              |  <- inline storage (32 bytes, aligned to 8)
|  void* heap              |  <- heap pointer (null when using SBO)
+--------------------------+
```

| Accessor | Job |
|---|---|
| `data()` | Address of the active callable (inline or heap) |
| `inlineSlot()` | Address of the SBO buffer (for placement new) |
| `heapSlot()` | Address of the heap pointer (for ownership transfer) |

### VTable

Each callable type gets one `static constexpr VTable` instance shared across all wrappers holding that type. The vtable holds four function pointers:

```cpp
struct VTable {
    R    (*invoke) (CallableStorage&, Args&&...);
    void (*copy)   (CallableStorage&, const CallableStorage&);
    void (*move)   (CallableStorage&, CallableStorage&);
    void (*destroy)(CallableStorage&);
};
```

`MoveOnlyFunction` sets `copy` to `nullptr` — it is never called since copy operations are deleted.

### SBO decision

`SBOTraits<T>::fits` is a compile-time constant evaluated once per callable type `T`:

```cpp
sizeof(T)  <= 32 &&
alignof(T) <= 8
```

All `if constexpr (SBOTraits<T>::fits)` branches in `VTableFactory` are resolved at compile time — there is no runtime branching per invocation to determine storage location.

### FunctionRef

`FunctionRef` does not use `CallableStorage` or `VTableFactory`. It stores two members:

```
+-----------------------------------+
|  PtrStorage ptr_                  |  8 bytes (union: object ptr or fn ptr)
+-----------------------------------+
|  R (*invoke_)(PtrStorage, Args&&) |  8 bytes (function pointer thunk)
+-----------------------------------+
```

Total: 16 bytes. No allocation, no vtable, no ownership.

A `union { void* obj; void (*fn)(); }` is used for storage because `void*` cannot portably hold a function pointer. The correct union member is selected at construction time based on whether `T` is a function type or a callable object.

### Owning wrapper layout

```
+-----------------------------------+
|  const VTable<R,Args...>* vtable_ |  8 bytes
+-----------------------------------+
|  CallableStorage storage_         |  40 bytes (32 buffer + 8 heap ptr)
+-----------------------------------+
```

Total: 48 bytes. `vtable_ == nullptr` is the canonical empty state.

---

## Complexity

### Time complexity

| Operation | Complexity | Notes |
|---|---|---|
| Construction (SBO) | O(1) | Placement construction into inline buffer |
| Construction (heap) | O(1) | Single heap allocation + construction |
| Invocation | O(1) | One indirect function pointer call |
| Copy (SBO) | O(1) | Placement copy-construction into inline buffer |
| Copy (heap) | O(1) | Single heap allocation + copy-construction |
| Move (SBO) | O(1) | Placement move-construction into inline buffer |
| Move (heap) | O(1) | Pointer transfer — no allocation |
| Destruction (SBO) | O(1) | Destructor call only |
| Destruction (heap) | O(1) | Destructor + single heap deallocation |
| `reset()` | O(1) | Destroy + null vtable and heap pointer |
| `operator bool` | O(1) | Null vtable check |
| `FunctionRef` construction | O(1) | Address capture only — no allocation |
| `FunctionRef` invocation | O(1) | One indirect function pointer call |

### Space complexity

| Type | Size | Notes |
|---|---|---|
| `Function` | 48 bytes | 8 vtable ptr + 32 buffer + 8 heap ptr |
| `MoveOnlyFunction` | 48 bytes | Same layout as `Function` |
| `FunctionRef` | 16 bytes | 8 ptr union + 8 thunk ptr |

---

## Benchmarks

Benchmarks compare `Function`, `MoveOnlyFunction`, and `FunctionRef` against `std::function` across construction, invocation, copy, and move operations.

> Compiled with `-std=c++23`. Results may vary depending on hardware and compiler optimizations.

<details>
<summary>Show benchmark results</summary>

#### Function

```
----------------------------------------------------------------------
Function Benchmarks                     Time           Iteration
----------------------------------------------------------------------
Construct SBO
Function Construct SBO                  4.02 ms         500000
Std::function Construct SBO             4.77 ms         500000

Construct Heap
Function Construct Heap                 97.48 ms        500000
Std::function Construct Heap            64.48 ms        500000

Invoke Free Function
Function Invoke Free                    3.71 ms         1000000
Std::function Invoke Free               4.23 ms         1000000

Invoke SBO
Function Invoke SBO                     4.23 ms         1000000
Std::function Invoke SBO                3.92 ms         1000000

Invoke Heap
Function Invoke Heap                    3.18 ms         1000000
Std::function Invoke Heap               3.71 ms         1000000

Copy SBO
Function Copy SBO                       4.37 ms         500000
Std::function Copy SBO                  4.77 ms         500000

Copy Heap
Function Copy Heap                      163.58 ms       500000
Std::function Copy Heap                 69.32 ms        500000

Move SBO
Function Move SBO                       3.97 ms         500000
Std::function Move SBO                  6.60 ms         500000

Move Heap
Function Move Heap                      63.79 ms        500000
Std::function Move Heap                 64.30 ms        500000

MoveOnlyFunction Vs Function
MoveOnlyFunction Move SBO               3.97 ms         500000
Function Move SBO                       3.96 ms         500000

FunctionRef Vs Function Vs Std::function
FunctionRef Invoke                      539.77 us       1000000
Function Invoke                         4.23 ms         1000000
Std::function Invoke                    4.02 ms         1000000
----------------------------------------------------------------------
```

#### MoveOnlyFunction

```
----------------------------------------------------------------------
MoveOnlyFunction Benchmarks             Time           Iteration
----------------------------------------------------------------------
Construct SBO
MoveOnlyFunction Construct SBO          4.03 ms         500000
Function Construct SBO                  3.97 ms         500000

Construct Heap
MoveOnlyFunction Construct Heap         63.35 ms        500000
Function Construct Heap                 63.36 ms        500000

Construct Move-Only Callable
Construct Unique Ptr Capture            63.20 ms        500000

Invoke Free Function
MoveOnlyFunction Invoke Free            3.73 ms         1000000
Function Invoke Free                    3.89 ms         1000000

Invoke SBO
MoveOnlyFunction Invoke SBO             4.23 ms         1000000
Function Invoke SBO                     3.70 ms         1000000

Invoke Heap
MoveOnlyFunction Invoke Heap            3.18 ms         1000000
Function Invoke Heap                    3.76 ms         1000000

Invoke Move-Only Callable
Invoke Move-only Capture                3.72 ms         1000000

Move SBO
MoveOnlyFunction Move SBO               3.97 ms         500000
Function Move SBO                       3.97 ms         500000

Move Heap
MoveOnlyFunction Move Heap              64.12 ms        500000
Function Move Heap                      63.98 ms        500000

Move Unique Ptr Capture
Move Unique Ptr Capture                 120.83 ms       500000

Reset SBO
MoveOnlyFunction Reset SBO              2.35 ms         500000
Function Reset SBO                      1.84 ms         500000

Reset Heap
MoveOnlyFunction Reset Heap             3.17 ms         500000
Function Reset Heap                     3.17 ms         500000
----------------------------------------------------------------------
```

#### FunctionRef

```
----------------------------------------------------------------------
FunctionRef Benchmarks                  Time           Iteration
----------------------------------------------------------------------
Construct Functor
FunctionRef Construct                   1.05 ms         1000000
Function Construct                      7.92 ms         1000000
Std::function Construct                 9.06 ms         1000000

Construct Free Function
FunctionRef Construct Free Function     1.07 ms         1000000
Function Construct Free Function        7.94 ms         1000000
Std::function Construct Free Function   10.55 ms        1000000

Invoke Functor
FunctionRef Invoke Functor              526.38 us       1000000
Function Invoke Functor                 3.71 ms         1000000
Std::function Invoke Functor            4.23 ms         1000000

Invoke Free Function
FunctionRef Invoke Free Function        526.38 us       1000000
Function Invoke Free Function           4.24 ms         1000000
Std::function Invoke Free Function      4.54 ms         1000000

Invoke Lambda
FunctionRef Invoke Lambda               526.46 us       1000000
Function Invoke Lambda                  3.70 ms         1000000
Std::function Invoke Lambda             4.39 ms         1000000

Copy
FunctionRef Copy                        1.05 ms         1000000

Rebind Vs Reassign
FunctionRef Rebind                      526.46 us       1000000
Function Reassign                       7.92 ms         1000000

Pass By Value
FunctionRef Pass By Value               1.05 ms         1000000
----------------------------------------------------------------------
```

#### Summary

`Function` matches or outperforms `std::function` on invocation and SBO operations. Heap copy is slower than `std::function` — this is an expected tradeoff of a simpler allocator strategy without the small-allocator optimizations some standard library implementations apply.

`MoveOnlyFunction` is identical in performance to `Function` for all move and invocation operations, with no overhead from the deleted copy path.

`FunctionRef` is the standout: construction is 7-9x faster than `Function` or `std::function`, and invocation is roughly 7-8x faster — down to ~526 ns per million calls vs ~4 ms — due to eliminating the vtable indirection and null check on the hot invocation path.

| Category | Winner | Notes |
|---|---|---|
| SBO construction | Function | Comparable to std::function |
| Heap construction | std::function | std::function uses optimized allocator on some impls |
| Invoke (all types) | FunctionRef | ~7x faster — no vtable, no null check |
| SBO copy | Function | Comparable to std::function |
| Heap copy | std::function | Same reason as heap construction |
| SBO move | Function | 1.66x faster than std::function |
| Heap move | Tie | Pointer transfer cost is identical |
| Construction (ref) | FunctionRef | 7-9x faster — address capture only |
| Rebind vs reassign | FunctionRef | 15x faster — no allocation or destruction |

**Use `Function` when:** you need a copyable callable with ownership and SBO.
**Use `MoveOnlyFunction` when:** your callable captures move-only types like `unique_ptr`.
**Use `FunctionRef` when:** you need to pass a callable by reference with zero overhead and can guarantee the callable's lifetime.

</details>

---

## Project Structure

```
FunctionPro/
├── include/
│   └── FunctionPro/
│       ├── Function.h
│       ├── Function.tpp
│       ├── MoveOnlyFunction.h
│       ├── MoveOnlyFunction.tpp
│       ├── FunctionRef.h
│       ├── FunctionRef.tpp
│       └── Detail/
│
├── tests/
├── benchmarks/
├── examples/
│
├── cmake/
│   └── FunctionProConfig.cmake.in
│
├── docs/
│   └── Architecture.md
│
├── .gitignore
├── CMakeLists.txt
├── README.md
└── LICENSE
```

---

## Building from Source

### Requirements

- GCC 13+ or Clang with C++23 support
- CMake 3.20+

### Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Run tests

```bash
./tests              # run all test suites
./tests list          # list available suites
./tests 1             # run by number
./tests constructor   # run by name
```

### Run benchmarks

```bash
./benchmarks           # run all benchmark suites
./benchmarks list       # list available suites
./benchmarks 1          # run by number
./benchmarks put        # run by name
```

### Run examples

```bash
./examples              # run all examples
./examples list          # list available examples
./examples 1             # run by number
./examples basic_usage   # run by name
```

---

## Known Limitations

- **`FunctionRef` does not own the callable it wraps.** The referenced object must outlive the `FunctionRef`; binding to a temporary is rejected at compile time, but a dangling reference is still possible if the underlying callable is destroyed while a `FunctionRef` to it is still in use.
- **`Function` requires copy-constructible callables.** Move-only callables (e.g. those capturing a `unique_ptr`) must use `MoveOnlyFunction` instead.
- **Heap-path construction and copy are slower than `std::function`** (e.g. `Function Construct Heap`: `97.48 ms` vs `64.48 ms`; `Function Copy Heap`: `163.58 ms` vs `69.32 ms`, both at 500,000 iterations) — some standard library implementations apply small-allocator optimizations that this project's simpler heap allocation strategy doesn't replicate. See [Benchmarks](#benchmarks).
- **`MoveOnlyFunction::operator()` is non-`const`**, while `Function::operator()` is `const`-qualified — this intentionally matches `std::move_only_function` and `std::function` semantics respectively, but means the two types aren't drop-in interchangeable in const contexts.

---

## License

Licensed under the [MIT License](LICENSE) — free to use, modify, and distribute for educational and personal purposes.

