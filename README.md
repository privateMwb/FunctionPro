# FunctionPro

[![C++23](https://img.shields.io/badge/C%2B%2B-17-blue)](https://en.cppreference.com/w/cpp/17)
[![Status](https://img.shields.io/badge/status-learning%20project-green)](https://github.com/privateMwb/FunctionPro)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A custom C++ callable wrapper library built for learning type erasure, small buffer optimization, and vtable-based dispatch — featuring three wrapper classes with distinct ownership models.

---

## Table of Contents

- [Overview](#overview)
- [Motivation / Goals](#motivation--goals)
- [Features](#features)
- [Design Overview](#design-overview)
  - [Internal Structure](#internal-structure)
  - [SBO Strategy](#sbo-strategy)
  - [Vtable Dispatch](#vtable-dispatch)
  - [Ownership Models](#ownership-models)
  - [Exception Safety Model](#exception-safety-model)
  - [Design Philosophy](#design-philosophy)
- [Complexity](#complexity)
  - [Time Complexity](#time-complexity)
  - [Space Complexity](#space-complexity)
- [Quick Example](#quick-example)
  - [Function](#function-example)
  - [MoveOnlyFunction](#moveonlyfunction-example)
  - [FunctionRef](#functionref-example)
- [Core API](#core-api)
  - [Function](#function)
  - [MoveOnlyFunction](#moveonlyfunction)
  - [FunctionRef](#functionref)
- [Project Structure](#project-structure)
- [Build Instructions](#build-instructions)
- [Notes](#notes)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

FunctionPro is a `std::function`-like callable wrapper library implemented from scratch in modern C++ (C++17).  
It focuses on understanding how type erasure works internally, including SBO, vtable dispatch, and ownership semantics.

It includes three wrapper classes:

- `Function` — copyable and movable callable wrapper with SBO
- `MoveOnlyFunction` — move-only callable wrapper with SBO, supports non-copyable callables
- `FunctionRef` — non-owning, non-allocating view of a callable

It also includes:

- Vtable-based type erasure via `VTableFactory`
- Small Buffer Optimization (SBO) with heap fallback
- Benchmark suite comparing against `std::function`
- Unit tests for correctness validation

---

## Motivation / Goals

This project was built to understand:

- Type erasure via vtables and function pointers
- Small Buffer Optimization (SBO) to avoid heap allocation
- Rule of 5 (copy/move semantics) in generic wrappers
- Ownership models — owning vs non-owning callables
- SFINAE-based constructor constraints with `enable_if_t`
- Performance benchmarking vs `std::function`

---

## Features

- Wraps free functions, lambdas, capturing lambdas, and callable objects
- Small Buffer Optimization — avoids heap allocation for small callables
- Heap fallback for large callables
- `Function` — full copy and move support
- `MoveOnlyFunction` — move-only, supports `unique_ptr` captures and non-copyable types
- `FunctionRef` — zero allocation, non-owning, rebindable
- `operator bool` for empty state checks
- `reset()` to clear stored callable
- `operator==` / `operator!=` with `nullptr`
- `std::bad_function_call` on invocation of empty wrapper

---

## Design Overview

FunctionPro uses a shared `Storage` struct with an SBO buffer and a heap pointer, combined with a vtable of function pointers for type-erased operations.

### Internal Structure

```
Storage
  ├── buffer[32]   ← SBO region (stack-allocated)
  └── heap*        ← heap pointer (null if SBO used)

VTable<R, Args...>
  ├── invoke(void*, Args&&...)
  ├── copy(void*, const void*)
  ├── move(void*, void*, bool)
  └── destroy(void*)
```

- `buffer` → 32-byte aligned stack region for small callables
- `heap` → heap pointer, null when SBO is active
- `vtable_` → pointer to a static `VTable` instance for the stored type

### SBO Strategy

When a callable fits within the SBO buffer:

```cpp
sizeof(T)  <= Storage::SBO_SIZE      // 32 bytes
alignof(T) <= Storage::SBO_ALIGNMENT //  8 bytes
```

The callable is constructed directly into the buffer with placement new — no heap allocation occurs.  
Otherwise, the callable is heap-allocated and the pointer is stored in `Storage::heap`.

```
Small callable  →  buffer[32]     (no allocation)
Large callable  →  new T(...)     (heap fallback)
```

### Vtable Dispatch

Each wrapper holds a pointer to a `VTable` populated by `VTableFactory`:

```cpp
static const VTable<R, Args...>* get() noexcept {
    static constexpr VTable<R, Args...> vtable {
        &invoke, &copy, &move, &destroy
    };
    return &vtable;
}
```

The vtable is a static constexpr instance — shared across all wrappers holding the same type.  
`MoveOnlyFunction` uses `getMoveOnly()` which sets `copy` to `nullptr`, preventing accidental copies.

### Ownership Models

| Class              | Owns Callable | Copyable | Movable | Allocates |
| ------------------ | ------------- | -------- | ------- | --------- |
| `Function`         | Yes           | Yes      | Yes     | Heap fallback |
| `MoveOnlyFunction` | Yes           | No       | Yes     | Heap fallback |
| `FunctionRef`      | No            | Yes      | Yes     | Never     |

`FunctionRef` stores only two pointers — the callable address and the invoke function pointer.  
It never allocates and the caller is responsible for ensuring the callable outlives the ref.

### Exception Safety Model

- Strong safety in copy and move operations
- `std::bad_function_call` thrown on invocation of an empty wrapper
- Bounds and null checks via `operator bool` and `== nullptr`
- No memory leaks on destruction — `destroy` always called via vtable

### Design Philosophy

FunctionPro prioritizes:

- Learning type erasure internals
- Explicit control over allocation
- Performance awareness via SBO
- Minimal abstraction over raw vtable dispatch
- Understanding how `std::function` works under the hood

---

## Complexity

### Time Complexity

| Operation             | Function | MoveOnlyFunction | FunctionRef | Notes                          |
| --------------------- | -------- | ---------------- | ----------- | ------------------------------ |
| Construct (SBO)       | O(1)     | O(1)             | O(1)        | Placement new into buffer      |
| Construct (heap)      | O(1)     | O(1)             | —           | Single heap allocation         |
| Invoke                | O(1)     | O(1)             | O(1)        | One vtable/pointer indirection |
| Copy construct (SBO)  | O(1)     | —                | O(1)        | Copy into buffer               |
| Copy construct (heap) | O(1)     | —                | —           | Single heap allocation         |
| Move construct        | O(1)     | O(1)             | O(1)        | Pointer transfer               |
| Reset / Destroy       | O(1)     | O(1)             | O(1)        | Single destroy call            |

### Space Complexity

- `Function` / `MoveOnlyFunction`: O(1) — fixed 32-byte SBO buffer + heap pointer + vtable pointer
- `FunctionRef`: O(1) — two pointers only, no buffer

---

## Quick Example

### Function Example

```cpp
#include "function/Function.h"
#include <iostream>

using namespace FunctionPro;

int add(int a, int b) { return a + b; }

int main() {
    // free function
    Function<int(int, int)> f(add);
    std::cout << f(2, 3) << "\n"; // 5

    // lambda
    Function<int(int)> doubler([](int x) { return x * 2; });
    std::cout << doubler(5) << "\n"; // 10

    // copy
    Function<int(int)> copy(doubler);
    std::cout << copy(6) << "\n"; // 12

    // move
    Function<int(int)> moved(std::move(doubler));
    std::cout << moved(7) << "\n";           // 14
    std::cout << (doubler == nullptr) << "\n"; // 1

    // reset
    f.reset();
    std::cout << (f == nullptr) << "\n"; // 1

    return 0;
}
```

### MoveOnlyFunction Example

```cpp
#include "function/MoveOnlyFunction.h"
#include <iostream>
#include <memory>

using namespace FunctionPro;

int main() {
    // move-only callable (unique_ptr capture)
    auto ptr = std::make_unique<int>(99);
    MoveOnlyFunction<int()> f([p = std::move(ptr)]() { return *p; });
    std::cout << f() << "\n"; // 99

    // move
    MoveOnlyFunction<int()> moved(std::move(f));
    std::cout << moved() << "\n";         // 99
    std::cout << (f == nullptr) << "\n";  // 1

    return 0;
}
```

### FunctionRef Example

```cpp
#include "function/FunctionRef.h"
#include <iostream>

using namespace FunctionPro;

int add(int a, int b) { return a + b; }

int main() {
    // free function — no allocation
    FunctionRef<int(int, int)> ref(add);
    std::cout << ref(2, 3) << "\n"; // 5

    // lambda — must outlive the ref
    auto lam = [](int x) { return x * 4; };
    FunctionRef<int(int)> ref2(lam);
    std::cout << ref2(3) << "\n"; // 12

    // rebind
    auto lam2 = [](int x) { return x + 10; };
    ref2 = FunctionRef<int(int)>(lam2);
    std::cout << ref2(3) << "\n"; // 13

    return 0;
}
```

---

## Core API

### Function

```cpp
// Constructors & Destructor
Function();
Function(std::nullptr_t) noexcept;

template<typename T>
Function(T&& callable);

~Function();

Function(const Function& other);
Function& operator=(const Function& other);

Function(Function&& other) noexcept;
Function& operator=(Function&& other) noexcept;

// Invocation
R operator()(Args... args) const;

// State
explicit operator bool() const noexcept;
void reset() noexcept;

// Equality
bool operator==(std::nullptr_t) const noexcept;
bool operator!=(std::nullptr_t) const noexcept;
```

### MoveOnlyFunction

```cpp
// Constructors & Destructor
MoveOnlyFunction();
MoveOnlyFunction(std::nullptr_t) noexcept;

template<typename T>
MoveOnlyFunction(T&& callable);

~MoveOnlyFunction();

MoveOnlyFunction(const MoveOnlyFunction&)            = delete;
MoveOnlyFunction& operator=(const MoveOnlyFunction&) = delete;

MoveOnlyFunction(MoveOnlyFunction&& other) noexcept;
MoveOnlyFunction& operator=(MoveOnlyFunction&& other) noexcept;

// Invocation
R operator()(Args... args) const;

// State
explicit operator bool() const noexcept;
void reset() noexcept;

// Equality
bool operator==(std::nullptr_t) const noexcept;
bool operator!=(std::nullptr_t) const noexcept;
```

### FunctionRef

```cpp
// Constructors & Destructor
FunctionRef() noexcept;

template<typename T>
FunctionRef(T&& callable) noexcept;

~FunctionRef() = default;

FunctionRef(const FunctionRef&)            = default;
FunctionRef& operator=(const FunctionRef&) = default;

FunctionRef(FunctionRef&&) noexcept            = default;
FunctionRef& operator=(FunctionRef&&) noexcept = default;

// Invocation
R operator()(Args... args) const;

// State
explicit operator bool() const noexcept;

// Equality
bool operator==(std::nullptr_t) const noexcept;
bool operator!=(std::nullptr_t) const noexcept;
```

---

## Project Structure

```
FunctionPro/
├── include/
│   └── function/
│       ├── Function.h
│       ├── Function.tpp
│       ├── MoveOnlyFunction.h
│       ├── MoveOnlyFunction.tpp
│       ├── FunctionRef.h
│       ├── FunctionRef.tpp
│       └── detail/
│           ├── Storage.h
│           ├── SBOTraits.h
│           ├── VTable.h
│           └── VTableFactory.h
│
├── tests/
│   ├── test_helper.h
│   ├── test_function.cpp
│   ├── test_move_only_function.cpp
│   ├── test_function_ref.cpp
│   └── test_all.cpp
│
├── benchmarks/
│   ├── benchmark_helper.h
│   ├── benchmark_function.cpp
│   ├── benchmark_move_only_function.cpp
│   ├── benchmark_function_ref.cpp
│   └── benchmark_all.cpp
│
├── examples/
│   ├── example_function.cpp
│   ├── example_move_only_function.cpp
│   ├── example_function_ref.cpp
│   └── example_all.cpp
│
├── README.md
└── LICENSE
```

---

## Build Instructions

### Requirements

- C++23-compatible compiler: GCC 13+, Clang 17+, or MSVC 19.38+
- No external dependencies — header-only core library

### Compile & Run Tests

```bash
g++ -std=c++23 tests/*.cpp -Iinclude -o testsResult
./testsResult
```

### Compile & Run Benchmarks

```bash
g++ -std=c++23 -O2 benchmarks/*.cpp -Iinclude -o benchmarksResult
./benchmarkResult
```

> Use `-O2` or `-O3` for meaningful benchmark results. Debug builds distort timing significantly.

### Compile & Run Examples

```bash
g++ -std=c++23 examples/*.cpp -Iinclude -o examplesResult
./examplesResult
```

---

## Notes

- **Not production-ready.** This is an educational project — use `std::function` in real codebases.
- `FunctionRef` does not own the callable — the caller must ensure the callable outlives the ref.
- `MoveOnlyFunction` explicitly deletes copy construction and copy assignment.
- The SBO buffer is 32 bytes with 8-byte alignment — callables exceeding this fall back to heap.
- Exception safety is handled for core operations but may not match full STL guarantees in all edge cases.
- Iterator invalidation does not apply — these are non-container types.

---

## Contributing

Contributions, improvements, and learning-focused PRs are welcome! Some areas worth exploring:

- Allocator support for heap fallback
- `constexpr` callable support
- `noexcept` propagation from wrapped callable
- CMake build system integration
- CI pipeline (GitHub Actions)

---

## License

[MIT](LICENSE) — free to use, modify, and distribute for educational and personal purposes.
