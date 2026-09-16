# Google Test Suite

The same categories as `../custom/suite/` (see `../custom/suite/README.md`
for what each one covers), reimplemented with GoogleTest instead of the
custom framework. No `support/` here — GoogleTest supplies its own
registration (`TEST()`), assertions (`EXPECT_*`/`ASSERT_*`), and runner.

## Building

```
cmake -S . -B build_tests -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARKS=OFF -DBUILD_REGRESSION=OFF
cmake --build build_tests
```

This produces a `google_tests` executable under `build_tests/`, and
registers each `TEST()` as its own entry with CTest via
`gtest_discover_tests()`.

## Usage

Run every test in the executable:

```
./build_tests/google_tests
```

List all tests without running them — useful when you don't remember
a test's exact dnspro:

```
./build_tests/google_tests --gtest_list_tests
```

Run every test in one category by filtering on the `Category.` prefix,
e.g. everything under `Unit`:

```
./build_tests/google_tests --gtest_filter=Unit.*
```

Run a single test by its full `Category.TestDnsPro`:

```
./build_tests/google_tests --gtest_filter=Unit.BasicBehavior
```

The same category filter also works through ctest, since each `TEST()`
is registered as its own entry via `gtest_discover_tests()`:

```
ctest --test-dir build_tests -R Unit --output-on-failure
```
