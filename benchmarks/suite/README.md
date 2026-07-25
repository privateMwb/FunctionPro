# Benchmark Suite

This document describes the benchmark categories under `suite/` — what
each one measures, and the individual benchmarks it contains.

FunctionPro has three types, each benchmarked against its own std
counterpart: `Function` vs `std::function`, `MoveOnlyFunction` vs
`std::move_only_function`, and `FunctionRef` vs `std::function_ref`. A
file's default comparison is `FunctionPro` vs `std::function`; cases
covering `MoveOnlyFunction` or `FunctionRef` override the project labels
and header before their own benchmark calls, so a single file can cover
all three types without re-declaring the comparison each time.

Every `BENCH()` call, in every category below, is automatically repeated
at three iteration tiers — SMALL (10K), MEDIUM (100K), and LARGE (1M) —
to smooth out timing noise and show whether relative performance holds
steady as call volume increases. This applies uniformly across the whole
suite; it is not specific to any one category. The **Scaling** category
below measures something different: how per-operation cost changes as
capture-state size grows, independent of iteration count.

Some benchmarks run through `BENCH_SOLO()` instead of `BENCH()`, timing
FunctionPro alone. This happens for two different reasons, and each
occurrence says which applies:
- **Toolchain-gated** — `std::move_only_function` and `std::function_ref`
  aren't available on every toolchain yet (`std::function_ref` in
  particular is a C++26 feature, not yet shipped everywhere). These
  cases are wrapped in a feature-test `#if` and fall back to
  `BENCH_SOLO` when the std type isn't available.
- **Permanent** — no `#if` gate, because no std equivalent exists at
  all, regardless of toolchain version. For example, `FunctionRef`
  supports a null/empty state that `std::function_ref` doesn't (P0792's
  `function_ref` is non-nullable by design), so comparing null-equality
  checks against it isn't meaningful on any toolchain.

Benchmarks only split a case into multiple variants (e.g. small vs large
capture) when that split reflects a genuinely different code path and
cost — not just because a type-level distinction technically exists. A
split that would just re-run the same instructions under a different
label is left as a single case instead.

---

## Access

Benchmarks read and query operations on an already-bound instance —
invoking the stored callable, and checking whether one is held.

### Benchmarks

- `invoke.cpp` — `operator()` across all three types: the hit path
  against their std counterparts, the empty/throw path (all three throw
  `std::bad_function_call`), and `FunctionRef` bound to a raw function
  pointer vs a capturing lambda, isolating the function-pointer fast
  path it special-cases internally
- `bool_check.cpp` — `operator bool()` on a bound instance, across all
  three types against their std counterparts

---

## Core

Benchmarks the fundamental, most frequently exercised operations —
binding a callable, resetting an instance, and assigning between
instances.

### Benchmarks

- `bind.cpp` — construction from a callable, across all three types.
  `Function` and `MoveOnlyFunction` split small (SBO-fitting) vs large
  (heap-forcing) capture — genuinely different allocation paths.
  `FunctionRef` is a single case: it never copies the callable, so its
  bind cost is flat regardless of size
- `reset.cpp` — `reset()` on `Function` and `MoveOnlyFunction` only;
  `FunctionRef` has no `reset()` and is excluded entirely
- `assign.cpp` — copy-assign and move-assign across all three types.
  `Function` gets both (genuinely different costs — copy goes through
  copy-construct-then-swap, move goes through the vtable's move).
  `MoveOnlyFunction` gets move-assign only (copy-assign is deleted).
  `FunctionRef` gets a single "assign" case, not split into copy/move —
  both compile to the same trivial pointer-pair copy

---

## Lifecycle

Benchmarks object lifetime operations — construction, destruction,
copying, and moving — across small (SBO) vs large (heap) capture where
that split reflects a real cost difference.

### Benchmarks

- `construct_destroy.cpp` — default construction (empty, no callable)
  and destruction, across all three types against their std
  counterparts. No small/large split — an empty instance has nothing to
  size
- `copy_semantics.cpp` — copy construction. `MoveOnlyFunction` is
  excluded (copy constructor deleted). `Function` splits small vs large
  capture (SBO copy-constructs `T` inline; heap copy allocates and
  copies). `FunctionRef` stays a single case (trivial pointer-pair
  copy, flat regardless of size)
- `move_semantics.cpp` — move construction only (move-assign already
  lives in `core/assign.cpp`). `Function` and `MoveOnlyFunction` split
  small vs large capture — the interesting result here: heap move is a
  plain pointer swap (cheaper), while SBO move has to move-construct
  `T` into the new buffer, the opposite pattern from copy.
  `FunctionRef` stays a single case

---

## Scaling

Benchmarks how per-operation cost changes as capture-state size grows,
crossing the `SBO_SIZE` = 40-byte boundary (64-bit build) that decides
whether a callable stays inline or moves to the heap. This is a separate
axis from the SMALL/MEDIUM/LARGE iteration tiers described above: those
repeat the same fixed-size operation more times, while Scaling grows the
callable itself and observes the resulting cost.

`Function` only, in both files below. `MoveOnlyFunction` shares the
exact same `CallableStorage`/`SBOTraits`/`VTableFactory` backend for
this dispatch, so sweeping it too would just repeat the same shape under
a different label. `FunctionRef` has no such axis at all — it never
copies the callable, so its cost is flat regardless of size (see
`core/bind.cpp`).

### Benchmarks

- `capture_size.cpp` — construction cost at five capture sizes (0B,
  16B, 40B, 64B, 256B) against `std::function`. Expect a step up
  somewhere between 40B and 64B as construction starts allocating
- `invoke_by_size.cpp` — invoke cost at the same five capture sizes.
  Expect a flat line instead — invocation goes through a vtable
  indirect call regardless of where the callable physically lives, the
  opposite shape from construction's step

---

## Utility

Benchmarks helper and comparison operations that don't belong to any of
the categories above — swapping two instances, and comparing against a
null state.

### Benchmarks

- `swap.cpp` — `swap()` on `Function` and `MoveOnlyFunction` against
  their std counterparts. `FunctionRef` has no `swap()` member at all
  and is excluded entirely
- `null_compare.cpp` — `operator==(nullptr)` across all three types.
  `Function` and `MoveOnlyFunction` compare against their std
  counterparts; `FunctionRef` runs solo permanently (not toolchain-
  gated) since `std::function_ref` is non-nullable by design and has no
  equivalent
