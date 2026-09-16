# Custom Test Suite

Correctness tests for the library, using the project's own lightweight
framework (`RUN`, `CHK`, `CHK_THROWS`) instead of a third-party library.
Organized into categories under `suite/` (see `suite/README.md` for
what each category covers). Every suite registers itself automatically
at startup — no manual wiring needed when a new test file is added.

Output is colorized: green/red check and cross marks per test, timing
per test and per run, and failures with inline detail (the failing
expression, or an expected-vs-actual breakdown for `CHK_THROWS`).

## Building

Configure with tests enabled and build:

```
cmake -S . -B build_tests -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARKS=OFF -DBUILD_REGRESSION=OFF
cmake --build build_tests
```

This produces a single `tests` executable under `build_tests/`.

## Usage

Run every registered suite, then print pass/fail stats:

```
./build_tests/tests
```

```
Basic Behavior
  ✓ Test Pass                                    0.00ms
  ✗ Test Fail                                    0.00ms
      ✗ unit/basic_behavior.cpp:4  1 == 2

┌──────────────┐
│ Total  2     │
│ Passed 1     │
│ Failed 1     │
│ Rate   50%   │
│ Time   0.1ms │
└──────────────┘
```

List all suites, grouped by category as a tree — useful when you don't
remember a suite's id or exact file name:

```
./build_tests/tests list
```

```
Available test suites

Unit (8)
├─ [U1]  Access
├─ [U2]  Capacity
├─ [U3]  Comparison
├─ [U4]  Iterators
├─ [U5]  Modifiers
├─ [U6]  Observer
├─ [U7]  Search
└─ [U8]  Swap

┌───────────────┐
│ Suites     20 │
│ Categories 5  │
└───────────────┘
```

Run every suite in one category, e.g. everything under `unit`:

```
./build_tests/tests unit
```

Run a single suite by its id, e.g. the first Unit suite:

```
./build_tests/tests U1
```

Run a single suite by its file name instead of its id:

```
./build_tests/tests basic_behavior
```

Show the full command reference:

```
./build_tests/tests -h
./build_tests/tests --help
```

```
Usage:
  tests               run every suite and print stats
  tests list          list all suites, grouped by category
  tests <category>    run every suite in a category, e.g. unit
  tests <id>          run a single suite by id, e.g. U1
  tests <name>        run a single suite by file name, e.g. basic_behavior
  tests -h | --help   show this help message

Matching is case-insensitive.
```

Matching is case-insensitive. Category and file-name arguments are
matched against the same `list` output, so run `./build_tests/tests
list` first if you're not sure what's available.

The build also registers a `ctest` target, so `ctest --test-dir
build_tests` runs the same executable and reports pass/fail through
CTest instead.
