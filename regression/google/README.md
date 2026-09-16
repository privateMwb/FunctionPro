# Google Regression

Compares a Google Benchmark run against a saved baseline snapshot and
reports the change per benchmark — how much faster or slower the
current code is. Reads from
`benchmarks/baselines/<tag>/g<tag>.json` and
`benchmarks/results/google_benchmark_results.json`, and writes a JSON
and a markdown report back out to `benchmarks/results/`.

Set the tool's display label in `framework.h`'s `setProjectLabels()`
before use — it's what shows up in the exported markdown report's
title.

## Building

Configure with the regression tool enabled and build:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARKS=OFF -DBUILD_REGRESSION=ON
cmake --build build
```

This produces a single `google_regression` executable under `build/`,
and fetches `nlohmann_json` at configure time.

## Usage

Compare the newest baseline snapshot against the current benchmark
results, then export both reports:

```
./build/google_regression
```

List every available baseline snapshot, oldest to newest — useful
when you don't remember a baseline's exact name. Shows the project
name and a tree of version tags, with the total count boxed at the
bottom:

```
./build/google_regression list
```

List every baseline snapshot's available benchmarks, grouped by
starting letter with an aligned `[LetterN]` id next to each one (e.g.
`[A1]`) — useful when you don't remember a benchmark's exact name:

```
./build/google_regression methods
```

Restrict that listing to a single version:

```
./build/google_regression methods v1.2.0
```

Compare one named baseline against the current benchmark results:

```
./build/google_regression v1.2.0
```

Compare two named baselines directly against each other, instead of
against the current run. Both tags are baselines -- neither is the
live local run -- and both show up as the actual version tags in the
column headers (e.g. `v1.2.0` vs `v1.3.0`, instead of the generic
"Current"/"Baseline" labels used when comparing against a live local
run):

```
./build/google_regression v1.2.0 v1.3.0
```

Restrict any of the above to a single benchmark by adding its name
after the version tag(s). Works with the current-run comparison, a
single chosen baseline, or two baselines head-to-head:

```
./build/google_regression BM_PushBack
./build/google_regression v1.2.0 BM_PushBack
./build/google_regression v1.2.0 v1.3.0 BM_PushBack
```

A benchmark name containing spaces doesn't need quoting — every word
after the version tag(s) is joined back together into the name.

A benchmark can also be given as the `[LetterN]` id shown next to it
by `google_regression methods` (e.g. `a1`) instead of typing out its
full name, resolved against whichever baseline is being compared:

```
./build/google_regression v1.2.0 a1
```

If either side doesn't have the requested benchmark, the tool reports
which one and exits without writing a report.

Every run (except `list` and `methods`) writes
`google_regressions_results.json` and `google_regressions_results.md`
to `benchmarks/results/`, overwriting any previous report.

Run `google_regression -h` (or `--help`/`help`) to print this usage
summary from the command line.
