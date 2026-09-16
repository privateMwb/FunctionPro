# Tests

Correctness tests for the library, split into two independent suites:

- **`custom/`** — the project's own lightweight framework (`RUN`,
  `CHK`, `CHK_THROWS`), organized into categories with auto-registered
  suites. See `custom/README.md`.
- **`google_tests/`** — the same categories, reimplemented with
  GoogleTest. See `google_tests/README.md`.

Both build independently and both register with `ctest`, so
`ctest --test-dir build_tests` runs everything regardless of which
suite you're touching.

## Building

Configure with tests enabled and build everything:

```
cmake -S . -B build_tests -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARKS=OFF -DBUILD_REGRESSION=OFF
cmake --build build_tests
```

This produces two executables under `build_tests/`: `tests` (custom
framework) and `google_tests` (GoogleTest).

## Usage

See `custom/README.md` and `google/README.md` for each suite's
CLI. Both are also runnable together through ctest:

```
ctest --test-dir build_tests --output-on-failure
```
