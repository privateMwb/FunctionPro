# Regression

Tools for comparing a benchmark run against a saved baseline snapshot
and reporting the change per operation — how much faster or slower
the current code is. There are two implementations here, one per
benchmark framework this project uses:

- `custom/` — regression tool for the custom benchmark framework's
  suite. See `custom/README.md`.
- `google/` — regression tool for the Google Benchmark suite. See
  `google/README.md`.

Both are built the same way, gated behind the same
`-DBUILD_REGRESSION=ON` CMake option, and both write their JSON and
markdown reports to `results/`.
