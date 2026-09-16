# Examples

Runnable example programs demonstrating the library's usage.
Organized into categories under `suite/` (see `suite/README.md` for
what each category covers). Every example registers itself
automatically at startup — no manual wiring needed when a new example
file is added.

## Building

Configure with examples enabled and build:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=ON -DBUILD_BENCHMARKS=OFF -DBUILD_REGRESSION=OFF
cmake --build build
```

This produces a single `examples` executable under `build/`.

## Usage

Run every registered example in order, each under its own boxed
header, then print a timed run summary:

```
./build/examples
```

```
┌─ [Q1] Basic Behavior
│  quickstart
└─────────────────────
Creating a vector with 3 elements...
Pushing 4th element...
Size is now 4

┌─ [Q2] Custom Allocator
│  quickstart
└───────────────────────
Allocating from arena...

┌──────────────────┐
│ Examples       2 │
│ Categories     1 │
│ Time       0.4ms │
└──────────────────┘
```

Running a single example by id or name skips the summary box — it's
only useful once more than one example ran together (a full run, or a
whole category).

List all examples, grouped by category as a tree — useful when you
don't remember an example's id or exact file name:

```
./build/examples list
```

```
Available example suites

Quickstart (2)
├─ [Q1]  Basic Behavior
└─ [Q2]  Custom Allocator

┌──────────────┐
│ Suites     2 │
│ Categories 1 │
└──────────────┘
```

Run every example in one category, e.g. everything under `quickstart`
— same boxed-header-per-example output as a full run, with a summary
box at the end:

```
./build/examples quickstart
```

Run a single example by its id, e.g. the first quickstart example:

```
./build/examples Q1
```

Run a single example by its file name instead of its id:

```
./build/examples basic_behavior
```

Show the full command reference:

```
./build/examples -h
./build/examples --help
```

```
Usage:
  examples               run every example in order
  examples list          list all examples, grouped by category
  examples <category>    run every example in a category, e.g. quickstart
  examples <id>          run a single example by id, e.g. Q1
  examples <name>        run a single example by file name, e.g. basic_behavior
  examples -h | --help   show this help message

Matching is case-insensitive.
```

Matching is case-insensitive. Category and file-name arguments are
matched against the same `list` output, so run `./build/examples
list` first if you're not sure what's available.
