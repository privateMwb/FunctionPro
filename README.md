# FunctionPro

<p align="center">
  <img src="https://img.shields.io/github/v/release/privateMwb/FunctionPro?style=for-the-badge&logo=github&color=yellow" alt="Version">
  <img src="https://img.shields.io/badge/License-MIT-orange?style=for-the-badge" alt="License - MIT">
  <img src="https://img.shields.io/badge/C%2B%2B-23-blue?style=for-the-badge&logo=c%2B%2B" alt="C++ - 23">
</p>

<p align="center">
  <a href="https://github.com/privateMwb/FunctionPro/actions/workflows/build.yml">
    <img src="https://github.com/privateMwb/FunctionPro/actions/workflows/build.yml/badge.svg" alt="Build and Test">
  </a>
  <a href="https://github.com/privateMwb/FunctionPro/actions/workflows/benchmark.yml">
    <img src="https://github.com/privateMwb/FunctionPro/actions/workflows/benchmark.yml/badge.svg" alt="Benchmarks">
  </a>
  <a href="https://github.com/privateMwb/FunctionPro/actions/workflows/coverage.yml">
    <img src="https://github.com/privateMwb/FunctionPro/actions/workflows/coverage.yml/badge.svg" alt="Coverage">
  </a>
  <a href="https://github.com/privateMwb/FunctionPro/actions/workflows/sanitizers.yml">
    <img src="https://github.com/privateMwb/FunctionPro/actions/workflows/sanitizers.yml/badge.svg" alt="Sanitizers">
  </a>
  <a href="https://github.com/privateMwb/FunctionPro/actions/workflows/clang-tidy.yml">
    <img src="https://github.com/privateMwb/FunctionPro/actions/workflows/clang-tidy.yml/badge.svg" alt="Clang Tidy">
  </a>
  <a href="https://github.com/privateMwb/FunctionPro/actions/workflows/clang-format.yml">
    <img src="https://github.com/privateMwb/FunctionPro/actions/workflows/clang-format.yml/badge.svg" alt="Clang Format">
  </a>
  <a href="https://github.com/privateMwb/FunctionPro/actions/workflows/docs.yml">
    <img src="https://github.com/privateMwb/FunctionPro/actions/workflows/docs.yml/badge.svg" alt="Documentation">
  </a>
  <a href="https://github.com/privateMwb/FunctionPro/actions/workflows/release.yml">
    <img src="https://github.com/privateMwb/FunctionPro/actions/workflows/release.yml/badge.svg" alt="Release">
  </a>
  <a href="https://github.com/privateMwb/FunctionPro/actions/workflows/packaging.yml">
    <img src="https://github.com/privateMwb/FunctionPro/actions/workflows/packaging.yml/badge.svg" alt="Packaging">
  </a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/GCC-support-B46F1B?style=flat&logo=gnu" alt="GCC - support">
  <img src="https://img.shields.io/badge/Clang-support-045891?style=flat&logo=llvm" alt="Clang - support">
  <img src="https://img.shields.io/badge/MSVC-support-5C2D91?style=flat" alt="MSVC - support">
  <img src="https://img.shields.io/badge/AppleClang-support-000000?style=flat&logo=apple" alt="AppleClang - support">
</p>

FunctionPro is a header-only C++23 library of type-erased callable
wrappers — `Function`, `MoveOnlyFunction`, and `FunctionRef` — built
around a shared SBO storage core and a single-pointer vtable dispatch
model, with copy/move semantics and exception guarantees matching
their `std::function` family counterparts.

## 📑 Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Development](#development)
- [Benchmarks](#benchmarks)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [License](#license)

## <a id="features"></a>✨ Features

- **Union-reclaimed SBO buffer** — `CallableStorage` unions its
  inline buffer with the heap pointer instead of storing them
  side by side, so the 8 bytes that would otherwise sit unused
  whenever a callable is heap-allocated go toward inline capacity
  instead (40 bytes total on a 64-bit build, vs. a fixed 32-byte
  buffer plus a separate pointer field).
- **Static, per-type vtables** — `VTableFactory` produces one
  `static constexpr` `VTable` per concrete callable type, shared
  by every `Function`/`MoveOnlyFunction` instance holding that
  type. Binding a callable costs a single pointer store, not a
  per-instance vtable allocation.
- **Compile-time SBO placement, zero runtime branching** — whether
  a callable lives inline or on the heap is resolved entirely by
  `SBOTraits<T>::fits` at the call site that constructs the
  `VTable`. Invoke/copy/move/destroy never check "inline vs. heap"
  at runtime; the correct storage access is baked into the
  function pointers themselves.
- **`FunctionRef` skips the object-pointer indirection for function
  pointers** — free functions and function pointers are stored
  directly in the same union slot used for object addresses,
  saving one indirection on every call compared to always going
  through a stored object pointer.
- **Strong exception guarantee on `Function` copy-assignment** — the
  source is copied into a temporary before replacing `*this`, so a
  throwing copy constructor leaves the target completely
  unaffected, matching `std::function::operator=`.
- **Move-only support without giving up SBO** — `MoveOnlyFunction`
  wraps move-only callables (e.g. one capturing a
  `std::unique_ptr`) that `Function` can't, while still using the
  same inline/heap storage core.
- **Real move semantics in `swap()`** — swapping goes through the
  callable's actual move constructor via the vtable, not a raw
  byte swap of `CallableStorage`, so it stays correct for
  inline-stored callables holding a subobject with an internal
  self-pointer.

## <a id="requirements"></a>📋 Requirements

- A C++23-conformant compiler (tested: Clang, GCC, MSVC)
- CMake 3.20+

## <a id="installation"></a>📦 Installation

**From source:**

```bash
git clone https://github.com/privateMwb/FunctionPro.git
cd FunctionPro
cmake -B build \
  -DBUILD_TESTS=OFF \
  -DBUILD_BENCHMARKS=OFF \
  -DBUILD_TOOLS=OFF \
  -DBUILD_EXAMPLES=OFF
cmake --install build
```

Then, in your own `CMakeLists.txt`:

```cmake
find_package(FunctionPro CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE FunctionPro::FunctionPro)
```

> vcpkg and Conan packages are built and verified (recipe in
> `packaging/recipes/functionpro/`, port in
> `packaging/vcpkg/ports/functionpro/`), but not yet published to
> the public registries. This section will be updated once they are.

## <a id="quick-start"></a>🚀 Quick Start

```cpp
#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>
#include <FunctionPro/FunctionRef.h>

#include <memory>

using namespace FunctionPro;

int main() {
    // Function: copyable, SBO-backed, drop-in std::function replacement
    Function<int(int)> add = [](int x) { return x + 1; };
    int result = add(41); // 42

    // MoveOnlyFunction: wraps move-only captures Function can't
    auto owned = std::make_unique<int>(10);
    MoveOnlyFunction<int()> readOwned = [p = std::move(owned)] { return *p; };
    int value = readOwned(); // 10

    // FunctionRef: non-owning view — cheapest option when the
    // callable's lifetime is already guaranteed by the caller
    auto callable = [](int x) { return x * 2; };
    FunctionRef<int(int)> ref(callable);
    int doubled = ref(21); // 42

    // Invoking an empty Function/MoveOnlyFunction throws, like std::function
    Function<void()> empty;
    if (!empty)
        /* handle emptiness, or call it and catch std::bad_function_call */;
}
```

## <a id="project-structure"></a>🗂️ Project Structure

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
│           ├── CallableStorage.h
│           ├── SBOTraits.h
│           ├── VTable.h
│           └── VTableFactory.h
│
├── tests/
│   ├── support/
│   ├── suite/
│   ├── test_main.cpp
│   └── CMakeLists.txt
│
├── benchmarks/
│   ├── support/
│   ├── suite/
│   ├── baselines/
│   ├── bench_main.cpp
│   └── CMakeLists.txt
│
├── examples/
│   ├── support/
│   ├── suite/
│   ├── example_main.cpp
│   └── CMakeLists.txt
│
├── regression/
│   ├── support/
│   ├── regression_main.cpp
│   └── CMakeLists.txt
│
├── packaging/
│   ├── recipes/
│   │   └── functionpro/
│   ├── vcpkg/
│   │   └── ports/
│   │       └── functionpro/
│   └── vcpkg-smoke-test/
│
├── scripts/
│   └── update_package_files.py
│
├── .github/
│   ├── releases/
│   └── workflows/
│
├── cmake/
│   └── FunctionProConfig.cmake.in
│
├── docs/
│   ├── Doxyfile
│   └── README.md
│
├── .gitignore
├── CMakeLists.txt
├── README.md
└── LICENSE
```

## <a id="development"></a>🛠️ Development

The from-source install above builds the library only. To work on
FunctionPro itself — running tests, benchmarks, or the regression
tool — build with everything enabled (the default):

```bash
cmake -B build
cmake --build build
```

**Run the test suite:**

```bash
ctest --test-dir build
```

**Run benchmarks and check for regressions:**

```bash
./build/benchmarks
./build/regression                  # latest baseline vs. benchmarks/results/benchmark_results.json
./build/regression v1.2.0           # a specific baseline vs. current
./build/regression v1.2.0 v1.4.0    # two baselines against each other
```

`regression` picks the latest baseline by semantic version (`v1.10.0`
correctly outranks `v1.9.0`), not alphabetical filename order, and
auto-names its output (`regression_v1.2.0_vs_current.md`/`.json`, etc.).

See [packaging/README.md](packaging/README.md) for notes on verifying the vcpkg
port and Conan recipe locally.

## <a id="benchmarks"></a>📊 Benchmarks

Measured against `std::function`, `std::move_only_function`, and
`std::function_ref` at 10K / 100K / 1M iterations. `std::function_ref`
comparisons ran solo (`BENCH_SOLO`) on this toolchain, since
`std::function_ref` is a C++26 feature not yet shipped everywhere —
`FunctionRef` numbers are its own timings, not a delta. Full results:
`benchmarks/baselines/v1.0.0.md`.

| Operation | FunctionPro (1M) | std (1M) | Δ |
|---|---|---|---|
| Fn Move Construct (small) | 1.79 ms | 10.96 ms | +510.9% |
| Fn Move-assign | 1.50 ms | 5.04 ms | +236.7% |
| Fn Move Construct (large) | 13.41 ms | 23.71 ms | +76.9% |
| MoveOnlyFn Construct/destroy | 824.71 us | 1.36 ms | +64.8% |
| MoveOnlyFn Move-assign | 1.63 ms | 2.46 ms | +50.3% |
| Fn::operator() (empty) | 714.73 ms | 1.04 s | +45.7% |
| Fn Bind (small) | 2.08 ms | 2.91 ms | +40.1% |
| Fn Copy Construct (small) | 2.73 ms | 3.54 ms | +29.6% |
| MoveOnlyFn Move Construct (small) | 1.91 ms | 2.45 ms | +28.1% |
| Fn::reset() | 12.60 ms | 13.09 ms | +3.9% |
| Fn::operator() | 1.39 ms | 1.36 ms | -2.2% |
| Fn::operator==(nullptr) | 286.32 us | 270.72 us | -5.4% |
| Fn Construct/destroy (empty) | 874.65 us | 818.60 us | -6.4% |
| Fn Copy-assign | 5.18 ms | 3.27 ms | -36.9% |
| Fn::swap() | 16.69 ms | 2.46 ms | -85.3% |

`Fn::*` rows compare against `std::function`; `MoveOnlyFn::*` rows
compare against `std::move_only_function`.

**`FunctionRef`** has no comparison table above since `std::function_ref`
(C++26) isn't shipped on this toolchain yet — these are solo timings,
not deltas:

| Operation | FunctionPro (1M) |
|---|---|
| FnRef::operator() | 2.04 ms |
| FnRef::operator() (empty) | 821.38 ms |
| FnRef::operator() (function pointer) | 1.64 ms |
| FnRef Move Construct | 541.39 us |
| FnRef Bind | 548.13 us |
| FnRef Copy Construct | 270.72 us |
| FnRef Construct/destroy (empty) | 270.72 us |
| FnRef Assign | 270.72 us |
| FnRef::operator Bool() | 270.72 us |
| FnRef::operator==(nullptr) | 270.71 us |

Notably, `FnRef::operator()` bound to a function pointer (1.64 ms)
beats a lambda-bound call (2.04 ms) — the function-pointer fast path
(see Features) skipping the object-pointer indirection shows up
directly in the numbers.

FunctionPro's biggest wins are in moving and empty-call handling:
heap-stored moves are a plain pointer transfer rather than a
reallocation, and an empty call fails fast through a single vtable
check rather than whatever internal branching the std counterparts
use for the same case.

The trade-off is in the two operations where FunctionPro does
deliberately more work for correctness. `swap()` always goes through
the callable's real move constructor instead of swapping raw storage
bytes, so it stays correct for callables with an internal
self-pointer into their own storage — that guarantee costs real time
next to a simpler byte-swap. `Copy-assign` builds the copy in a
temporary before touching `*this`, so a throwing copy constructor
leaves the destination untouched — the strong exception guarantee,
paid for on every copy-assign, not just the ones that throw.

## <a id="documentation"></a>📖 Documentation

Full API reference, generated with Doxygen from `docs/Doxyfile`:

**https://privateMwb.github.io/FunctionPro/**

## <a id="contributing"></a>🤝 Contributing

Issues and pull requests are welcome. Before submitting a PR:

- Run the test suite (`ctest --test-dir build`)
- If you're changing a hot path, run `./build/regression` and mention
  the results in your PR description

## <a id="changelog"></a>📝 Changelog

See the [Releases](https://github.com/privateMwb/FunctionPro/releases)
page for version history and release notes.

## <a id="license"></a>📄 License

MIT — see [LICENSE](LICENSE) for details.
