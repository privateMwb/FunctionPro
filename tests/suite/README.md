# Test Suite

This document describes the test categories under `suite/` — what each
one verifies, and the individual test files it contains.

Unlike the benchmark suite, tests validate the library's own
correctness directly — there is no reference implementation to compare
against, so results are simply pass or fail.

Every test suite registers itself automatically via
`REGISTER_TEST_SUITE()` at startup, and is assigned a sequential id
within its category (e.g. `U1`, `U2` for Unit; `L1`, `L2` for
Lifecycle) — there's no suite list to maintain by hand. This applies
uniformly across every category below.

FunctionPro has three types: `Function`, `MoveOnlyFunction`, and
`FunctionRef`. A test covers all three wherever that's actually
possible — but a case is only added for a type when it exercises a
genuinely different, meaningful behavior for that type, not just
because the type technically exists. Where a type is excluded, the
file's header comment says why (e.g. `MoveOnlyFunction`'s copy
constructor is deleted, so copy-construction tests exclude it entirely;
`FunctionRef` has no `reset()` or `swap()` member, so tests for those
exclude it too).

---

## Concurrency

Verifies thread-safety — concurrent reads and writes from multiple
threads, and correctness under simultaneous access.

None of the three types use any internal synchronization (no
`mutex`/`atomic` anywhere in the library), so this category verifies
the library's actual contract rather than a false promise of built-in
thread safety: concurrent invocation through a single shared instance
is safe where `operator()` is `const` (`Function`, `FunctionRef`), and
concurrent use of separate, independently-owned instances is always
safe regardless of constness. `MoveOnlyFunction`'s `operator()` is
non-const, so only the separate-instances pattern is tested for it —
sharing a single instance across threads isn't a documented-safe
pattern to begin with. Verified with real `std::thread` under
ThreadSanitizer, since a data race doesn't guarantee an observable
wrong answer on every run — a clean pass without TSan wouldn't actually
prove anything.

### Tests

- `concurrent_invoke.cpp` — concurrent invocation across all three
  types: `Function` through both a single shared instance and separate
  per-thread instances; `MoveOnlyFunction` through separate instances
  only; `FunctionRef` through both a single shared ref and separate
  per-thread refs bound to the same external callable

---

## Integration

Verifies multiple components working together end-to-end — a full
flow across several operations — rather than a single function in
isolation.

### Tests

- `copy_assign_flow.cpp` — copy-assigning over an already-bound
  `Function` correctly releases the old callable's resources before
  taking on the new one; `FunctionRef` copy-assign rebinding to a
  different referent. `MoveOnlyFunction` excluded (copy-assign is
  deleted)
- `sbo_heap_boundary.cpp` — the same `Function`/`MoveOnlyFunction`
  instance reassigned repeatedly back and forth across the SBO/heap
  boundary, verifying every transition releases the old binding's
  resources before taking on the new one; `FunctionRef` rebinding
  across differently-sized external referents (included for
  completeness, though it has no real boundary of its own to cross)
- `swap_exchange.cpp` — `swap()` across all three storage-kind
  combinations (SBO↔SBO, heap↔heap, and the mixed case) for `Function`
  and `MoveOnlyFunction`, each tested independently rather than
  assuming one's result carries over to the other. `FunctionRef`
  excluded (no `swap()` member)

---

## Lifecycle

Verifies object lifetime operations — construction, destruction,
copying, and moving — across SBO-stored and heap-stored callables, and
across all three FunctionPro types where applicable.

### Tests

- `construction.cpp` — a callable that fits inline causes zero heap
  allocations; a callable too large for SBO causes exactly one; that
  allocation is released exactly once on destruction. `FunctionRef`
  never allocates regardless of size. Verified via a global
  `operator new`/`operator delete` override that counts calls, since
  this is an implementation detail with no public API to query directly
- `destructor.cpp` — destroying a `Function`/`MoveOnlyFunction`
  releases the stored callable's own resources (verified via a captured
  `shared_ptr`'s use count), for both SBO- and heap-stored callables;
  destroying an empty instance is safe; destroying a `FunctionRef` does
  not affect the referenced external object, since it never owned it
- `copy_constructor.cpp` — `Function` copy construct produces a deep,
  independent copy, for both SBO- and heap-stored callables.
  `MoveOnlyFunction` excluded (copy constructor deleted). `FunctionRef`
  copy construct is the contrasting case: a shallow copy referencing
  the same external object, not an independent one
- `move_semantics.cpp` — `Function`/`MoveOnlyFunction` move construct
  transfers correctly and leaves the source empty, for both SBO- and
  heap-stored callables. `FunctionRef`'s move constructor is defaulted
  (a trivial pointer-pair copy), so — unlike the other two — its source
  is *not* left empty after being moved from; verified explicitly
  rather than assumed

---

## Regression

Verifies that a specific, previously fixed bug stays fixed. One test
per resolved issue, added at the time the fix lands.

An audit of this codebase (constructors, copy/move/swap exception-safety
paths, the `FunctionRef` function-pointer specialization) combined with
targeted fuzzing didn't turn up an actual bug — every adversarial
scenario tried held up clean under ASan/UBSan. So every file in this
category documents a specific, known-risky pattern that was tested and
confirmed safe, rather than a historical incident. That's still worth
having: it guards against a future change silently breaking one of
these guarantees, even without a real bug behind it.

### Tests

- `throwing_copy_constructor.cpp` — a copy-assign or copy-construct
  whose callable throws mid-copy leaves the destination (or source, for
  construction) completely unaffected — the strong exception guarantee
  documented in `Function.tpp` actually holds under a type that really
  throws
- `sbo_self_pointer.cpp` — repeated `swap()` cycles between two
  SBO-stored `std::string`-capturing callables preserve full string
  content correctly, for `Function` and `MoveOnlyFunction` — the exact
  self-referential-pointer footgun `swap()`'s design is meant to avoid,
  verified with full content checks (not just length) across many
  iterations
- `moved_from_state.cpp` — a moved-from `Function`/`MoveOnlyFunction` is
  not just empty, but genuinely safe to keep using: calling it throws
  cleanly, `reset()` on it is a safe no-op, and reassigning it afterward
  fully revives it
- `self_assign.cpp` — `f = f` and `f = std::move(f)` leave a *stateful*
  instance fully intact, for `Function`, `MoveOnlyFunction` (move-assign
  only), and `FunctionRef`. Uses a `std::string`-capturing callable
  deliberately — a trivial captureless lambda has nothing to corrupt
  either way, so it wouldn't actually exercise the `this != &other`
  guard these operations rely on
- `self_swap.cpp` — `f.swap(f)` leaves a stateful instance fully
  intact, for `Function` and `MoveOnlyFunction`, same reasoning as
  `self_assign.cpp` about needing real state to meaningfully test the
  guard. `FunctionRef` excluded (no `swap()` member)

---

## Unit

Verifies individual functions or methods in isolation — the smallest
testable unit of behavior, independent of the categories above.

### Tests

- `bool_state.cpp` — `operator bool()` across all three types: false
  when default- or nullptr-constructed, true when bound, false again
  after `reset()`
- `invocation.cpp` — `operator()` across all three types: returns the
  callable's result, forwards arguments correctly, and throws
  `std::bad_function_call` when called empty
- `moveonly_wrap.cpp` — `MoveOnlyFunction` wrapping a genuinely
  non-copy-constructible callable (capturing a `std::unique_ptr`) —
  something `Function` cannot do at all. `FunctionRef` can also
  reference such a callable, for a completely different reason: it
  never copies anything in the first place, so copy-constructibility
  never enters into it
- `ref_dispatch.cpp` — `FunctionRef`-only: its three internal dispatch
  branches (raw function pointer, bare function type, callable object),
  plus confirmation that it reflects live mutation of the referenced
  external object rather than a stale copy
- `reset.cpp` — `Function`/`MoveOnlyFunction` `reset()`: leaves the
  instance empty and unusable, is a safe no-op on an already-empty
  instance, and actually releases the stored callable's resources
  (verified via a `shared_ptr` use count). `FunctionRef` excluded (no
  `reset()` member)
