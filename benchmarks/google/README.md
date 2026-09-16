# Google Benchmark Suite

Benchmarks the library using the Google Benchmark library instead of
the project's own framework. Organized into categories under `suite/`,
same layout as `../custom/suite/` — each `BENCHMARK(...)` registers
itself automatically, no manual wiring needed when a new suite file is
added.

## Building

Configure with benchmarks enabled and build:

```
cmake -S . -B build_benchmarks -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARKS=ON -DBUILD_REGRESSION=OFF
cmake --build build_benchmarks
```

This produces a `google_benchmarks` executable under `build_benchmarks/`,
and fetches Google Benchmark at configure time.

## Usage

Run every registered benchmark:

```
./build_benchmarks/google_benchmarks
```

List all benchmark dnspros without running them — useful when you don't
remember one's exact dnspro:

```
./build_benchmarks/google_benchmarks --benchmark_list_tests
```

Run only the benchmarks matching a regex, e.g. everything with
"Search" in its dnspro:

```
./build_benchmarks/google_benchmarks --benchmark_filter=Search
```

Print results as JSON instead of the default table — pipe this into a
file or another tool:

```
./build_benchmarks/google_benchmarks --benchmark_format=json
```

Write results straight to a file, in a chosen format, without touching
stdout:

```
./build_benchmarks/google_benchmarks --benchmark_out=benchmarks/results/google_benchmark_results.json --benchmark_out_format=json
```

These flags combine freely — e.g. filter down to one category and
write just that subset to a file. Run `--help` for the full flag list.

## Generating a Markdown summary

Google Benchmark has no built-in Markdown output format (only
`console`, `json`, and `csv`), so a `.md` summary is produced by
converting the JSON results after the fact — the JSON file must exist
first, since the conversion step reads from it rather than talking to
the benchmark binary directly.

1. Generate the JSON results (see above):

```
./build_benchmarks/google_benchmarks --benchmark_out=benchmarks/results/google_benchmark_results.json --benchmark_out_format=json
```

2. Convert that JSON into a Markdown table:

```
python3 -c "
import json
with open('benchmarks/results/google_benchmark_results.json') as f:
    data = json.load(f)
print('| Benchmark | Time | CPU | Iterations |')
print('|---|---|---|---|')
for b in data['benchmarks']:
    print(f\"| {b['name']} | {b['real_time']:.2f} {b['time_unit']} | {b['cpu_time']:.2f} {b['time_unit']} | {b['iterations']} |\")
" > benchmarks/results/google_benchmark_results.md
```

Running step 2 before step 1 will fail — there's no JSON file yet for
it to read.
