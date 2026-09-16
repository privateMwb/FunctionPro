# Custom Suite

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
`std::move_only_function`, and `FunctionRef` vs `std::function_ref`. A
file's default comparison is `FunctionPro` vs `std::function`; cases
covering `MoveOnlyFunction` or `FunctionRef` override the project labels
and header before their own benchmark calls, so a single file can cover
all three types without re-declaring the comparison each time.

Every `BENCH()` call, in every category below, is automatically repeated at
three iteration tiers — SMALL (10K), MEDIUM (100K), and LARGE (1M) — to
smooth out timing noise and show whether relative performance holds steady
as call volume increases. This applies uniformly across the whole suite; it
is not specific to any one category. The **Scaling** category below measures
something different: how per-operation cost changes as capture-state size
grows, independent of iteration count.

Some benchmarks run through `BENCH_SOLO()` instead of `BENCH()`, timing
FunctionPro alone. This happens either because the std counterpart isn't
available on every toolchain yet (`std::move_only_function`,
`std::function_ref`) — gated behind a feature-test `#if` and falling back
to `BENCH_SOLO` — or because no std equivalent exists at all regardless of
toolchain, such as `FunctionRef`'s null/empty state, which `std::function_ref`
doesn't support by design.

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
| `assign.cpp` | Copy-assign and move-assign across all three types; `Function` gets both, `MoveOnlyFunction` move-assign only (copy-assign deleted), `FunctionRef` a single "assign" case |

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
| `move_semantics.cpp` | Move construction only (move-assign lives in `core/assign.cpp`), small vs large capture; `FunctionRef` stays a single flat case |

---

## Scaling

Benchmarks how per-operation cost changes as capture-state size grows,
crossing the `SBO_SIZE` = 40-byte boundary (64-bit build) that decides
whether a callable stays inline or moves to the heap — a separate axis from
the SMALL/MEDIUM/LARGE iteration tiers described above: those repeat the
same fixed-size operation more times, while Scaling grows the callable
itself and observes the resulting cost.

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
| `null_compare.cpp` | `operator==(nullptr)` across all three types; `FunctionRef` runs solo permanently — `std::function_ref` is non-nullable by design |

---

## Conventions

- **Headers** — `<support/framework.h>`, plus `<array>` in files using a
  `LargePayload`/`SizedCallable` struct to force heap allocation
- **Naming** — free functions prefixed `bench_`, one per case, e.g.
  `bench_function_bind_small`, `bench_move_only_swap`
- **Comparison macro** — `BENCH(label, cExpr, sExpr)` runs the FunctionPro
  lambda against the std lambda under a shared string `label`, at all three
  iteration tiers
- **Solo macro** — `BENCH_SOLO(label, cExpr)` runs only the FunctionPro
  side, for toolchain-gated or permanent cases (see above)
- **Timed region** — each case builds a `cExpr`/`sExpr` lambda pair
  capturing pre-built sources by reference; the lambda body is what
  `BENCH`/`BENCH_SOLO` times repeatedly, not the surrounding setup
- **doNotOptimize** — every timed lambda ends by passing its result through
  `doNotOptimize(v)` to prevent the optimizer from eliding the operation
  under test
- **Toolchain gating** — cases comparing against `std::move_only_function`
  or `std::function_ref` are wrapped in `#if defined(__cpp_lib_move_only_function)`
  / `#if defined(__cpp_lib_function_ref)`, falling back to `BENCH_SOLO` in
  the `#else` branch
- **Suite registration** — each file collects its `bench_*` calls into a
  single `run_benchmarks()` function, registered via `REGISTER_BENCH_SUITE()`
