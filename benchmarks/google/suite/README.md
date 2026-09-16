# Google Benchmark Suite

This document describes the benchmark categories under `suite/` — what each
one measures, and the individual benchmarks it contains.

| Category | Focus |
|---|---|
| [Access](#access) | Reads and queries on an already-bound instance |
| [Core](#core) | Binding a callable, resetting, and assigning between instances |
| [Lifecycle](#lifecycle) | Construction, destruction, copying, and moving |
| [Scaling](#scaling) | Cost vs. capture-state size, independent of iteration count |
| [Utility](#utility) | Swap and null-comparison overhead |

FunctionPro has three types, each benchmarked against its own std
counterpart: `Function` vs `std::function`, `MoveOnlyFunction` vs
`std::move_only_function`, and `FunctionRef` vs `std::function_ref`. Each
case is a pair of `BENCHMARK()` functions — one FunctionPro, one std —
sharing a common name stem (e.g. `Function_Bind_Small` /
`StdFunction_Bind_Small`) so the two show up next to each other in
benchmark output.

Iteration count is not fixed per file. Each `BENCHMARK()` case is a single
`for (auto _ : state)` loop; Google Benchmark calibrates how many times
that loop runs on its own (governed at run time by flags like
`--benchmark_min_time`), rather than repeating at fixed tiers. The
**Scaling** category below measures something different: how per-operation
cost changes as capture-state size grows, independent of iteration count.

Some benchmarks have no std-side function at all — just the FunctionPro
case, standalone. This happens either because the std counterpart isn't
available on every toolchain yet (`std::move_only_function`,
`std::function_ref`) — gated behind a feature-test `#if`, so the std
`BENCHMARK()` simply doesn't exist in the binary when unavailable — or
because no std equivalent exists at all regardless of toolchain, such as
`FunctionRef`'s null/empty state, which `std::function_ref` doesn't support
by design.

---

## Access

Benchmarks read and query operations on an already-bound instance —
invoking the stored callable, and checking whether one is held.

### Benchmarks

| File | What it covers |
|---|---|
| `invoke.cpp` | `operator()` hit path across all three types against their std counterparts, the empty/throw path (all three throw `std::bad_function_call`), and `FunctionRef` bound to a raw function pointer vs a capturing lambda |
| `bool_check.cpp` | `operator bool()` on a bound instance, across all three types against their std counterparts |

---

## Core

Benchmarks the fundamental, most frequently exercised operations —
binding a callable, resetting an instance, and assigning between instances.

### Benchmarks

| File | What it covers |
|---|---|
| `bind.cpp` | Construction from a callable, across all three types; `Function`/`MoveOnlyFunction` split small (SBO-fitting) vs large (heap-forcing) capture, `FunctionRef` is a single flat case |
| `reset.cpp` | `reset()` on `Function` and `MoveOnlyFunction` only — `FunctionRef` has no `reset()` and is excluded |
| `assign.cpp` | Copy-assign and move-assign across all three types; `Function` gets both, `MoveOnlyFunction` move-assign only (copy-assign deleted), `FunctionRef` a single "assign" case. Move-assign ping-pongs a populated instance between two slots inside the timed loop |

---

## Lifecycle

Benchmarks object lifetime operations — construction, destruction, copying,
and moving — across small (SBO) vs large (heap) capture where that split
reflects a real cost difference.

### Benchmarks

| File | What it covers |
|---|---|
| `construct_destroy.cpp` | Default construction (empty, no callable) and destruction, across all three types against their std counterparts |
| `copy_semantics.cpp` | Copy construction, small vs large capture (`MoveOnlyFunction` excluded — copy constructor deleted); `FunctionRef` stays a single flat case |
| `move_semantics.cpp` | Move construction only (move-assign lives in `core/assign.cpp`), small vs large capture; `FunctionRef` stays a single flat case. Each timed iteration rebuilds the source, since a moved-from instance is left empty |

---

## Scaling

Benchmarks how per-operation cost changes as capture-state size grows,
crossing the `SBO_SIZE` = 40-byte boundary (64-bit build) that decides
whether a callable stays inline or moves to the heap — a separate axis from
Google Benchmark's own iteration calibration described above: that repeats
the same fixed-size operation more times, while Scaling grows the callable
itself and observes the resulting cost. Each size point is a separate
`BENCHMARK_TEMPLATE(func, N)` instantiation, since capture size is a
compile-time template parameter (`SizedCallable<N>`); benchmark names carry
the size directly, e.g. `Function_Bind_BySize<64>`.

### Benchmarks

| File | What it covers |
|---|---|
| `capture_size.cpp` | Construction cost at five capture sizes (0B, 16B, 40B, 64B, 256B) against `std::function` |
| `invoke_by_size.cpp` | Invoke cost at the same five capture sizes |

---

## Utility

Benchmarks helper and comparison operations that don't belong to any of the
categories above — swapping two instances, and comparing against a null
state.

### Benchmarks

| File | What it covers |
|---|---|
| `swap.cpp` | `swap()` on `Function` and `MoveOnlyFunction` against their std counterparts — `FunctionRef` has no `swap()` member and is excluded |
| `null_compare.cpp` | `operator==(nullptr)` across all three types; `FunctionRef` runs standalone permanently — `std::function_ref` is non-nullable by design |

---

## Conventions

- **Headers** — `<benchmark/benchmark.h>` plus `<functional>` and whichever
  `<FunctionPro/*.h>` headers the file needs, instead of the custom suite's
  `<support/framework.h>`
- **Naming** — no `BM_` prefix. Pattern is `<Type>_<Operation>[_<Variant>]`,
  e.g. `Function_Bind_Small`, `StdMoveOnlyFunction_MoveAssign`,
  `FunctionRef_NullCompare`. The FunctionPro side and its std counterpart
  share the same stem so they sort next to each other in output
- **Registration** — every benchmark function is immediately followed by
  its own `BENCHMARK(...)` call (or `BENCHMARK_TEMPLATE(...)` for the
  Scaling category's compile-time size sweeps); nothing is registered
  through a suite-level runner function
- **Timed region** — setup happens before `for (auto _ : state) { ... }`;
  only the operation under test lives inside the loop
- **doNotOptimize** — `doNotOptimize(v)` becomes `benchmark::DoNotOptimize(v)`
- **Toolchain gating** — cases comparing against `std::move_only_function`
  or `std::function_ref` keep the same `#if defined(__cpp_lib_move_only_function)`
  / `#if defined(__cpp_lib_function_ref)` guards as the custom suite; when
  the guard is false, the std-side `BENCHMARK()` simply doesn't exist in
  the binary
- **No suite macros** — no `BENCH()`, `BENCH_SOLO()`, or
  `REGISTER_BENCH_SUITE()`. No `BENCHMARK_MAIN()` either — these files are
  linked against a separate translation unit that provides `main()`
