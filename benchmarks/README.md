# Benchmarks

Performance benchmarks for the library, split into two independent
suites:

- **`custom/`** — the project's own lightweight framework (`BENCH`,
  `BENCH_SOLO`, `BENCH_CUSTOM`), organized into categories with
  auto-registered suites. See `custom/README.md`.
- **`google/`** — the same categories, reimplemented with
  Google Benchmark. See `google/README.md`.

Both compare the library's core type against a reference
implementation under `shared/` — see each suite's `suite/README.md`
for what's actually being measured, category by category.

## Building

Configure with benchmarks enabled and build everything:

```
cmake -S . -B build_benchmarks -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARKS=ON -DBUILD_REGRESSION=OFF
cmake --build build_benchmarks
```

This produces two executables under `build_benchmarks/`: `benchmarks`
(custom framework) and `benchmarks_google` (Google Benchmark).

## Usage

See `custom/README.md` and `google/README.md` for each
suite's CLI — they don't share one, since Google Benchmark has its own
flag-based interface (`--benchmark_filter`, `--benchmark_out`, etc.)
rather than the custom framework's `list`/category/id/dnspro dispatch.

Once you have results, `../regression/` compares a run against a saved
baseline — see `../regression/README.md`.
