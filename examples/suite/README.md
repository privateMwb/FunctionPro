# Example Suite

This document describes the example categories under `suite/` — what
each one demonstrates, and the individual example files it contains.

Unlike the test suite, an example doesn't assert correctness — it
demonstrates real usage of the library, including deliberate misuse
where instructive (see Misuse), so the reader sees both the correct
pattern and the mistake it guards against.

Every example file exercises all three FunctionPro types — `Function`,
`MoveOnlyFunction`, and `FunctionRef` — side by side, so the reader can
see how each one handles the same situation, and where they diverge.
The one exception is `dangling_functionref.cpp`, whose central mistake
is specific to `FunctionRef`; it uses `Function` and `MoveOnlyFunction`
only for contrast, not as an equal focus.

Every example file ends with `REGISTER_EXAMPLE_SUITE()`, which derives
the suite's category from its containing directory and assigns it a
sequential id within that category. This applies uniformly across
every category below.

---

## Advanced

Demonstrates deeper mechanics of the library — move semantics,
exception safety, the inline/heap storage threshold, and why `swap()`
goes through the callable's real move constructor — for `Function`,
`MoveOnlyFunction`, and `FunctionRef` alike.

### Examples

- `move_semantics.cpp` — Moving `Function` and `MoveOnlyFunction`,
  moving a callable `Function` couldn't hold at all, and why moving a
  `FunctionRef` only moves the reference, never the referenced callable
- `exception_safety.cpp` — The strong exception guarantee on
  `Function`'s copy assignment when the source callable's copy
  constructor throws, contrasted with `MoveOnlyFunction` (no copy
  constructor to throw) and `FunctionRef` (a trivial, always-safe copy)
- `sbo_threshold.cpp` — Crossing `SBOTraits::fits` for `Function` and
  `MoveOnlyFunction` alike: a callable stored inline versus one that
  falls back to the heap, contrasted with `FunctionRef`, which never
  allocates at all
- `self_pointer_swap.cpp` — Why `Function::swap()` and
  `MoveOnlyFunction::swap()` move through the callable's real move
  constructor instead of swapping raw storage bytes, and why
  `FunctionRef` needs no such `swap()` to begin with

---

## Integration

Demonstrates interoperability with the rest of a codebase — the
standard algorithms library, embedding callbacks as class members, a
named-callback registry, and choosing between `Function`,
`MoveOnlyFunction`, and `FunctionRef`.

### Examples

- `stl_algorithms.cpp` — `FunctionRef` as a `std::sort`/`std::find_if`
  predicate, `Function` reused across `std::transform` and
  `std::accumulate`, and `MoveOnlyFunction` passed by move into
  `std::for_each`
- `class_member_storage.cpp` — Embedding a `Function` as a configurable
  callback member, a `MoveOnlyFunction` member that owns a resource, and
  `FunctionRef` accepted as a parameter for one call rather than stored
  as a member
- `callback_registry.cpp` — A map of named `Function` callbacks, a
  `MoveOnlyFunction`-based registry where handlers fire once and erase
  themselves, and a `FunctionRef`-based batch dispatch built and
  consumed within a single call
- `three_types_compared.cpp` — The same lambda bound to `Function`,
  `MoveOnlyFunction`, and `FunctionRef`, and when to choose each

---

## Misuse

Demonstrates common mistakes and the exceptions or undefined behavior
they lead to, alongside the correct pattern — including examples shown
but not executed, so the reader can see what to avoid without the
program actually invoking undefined behavior.

### Examples

- `dangling_functionref.cpp` — A `FunctionRef` outliving the callable it
  references, shown but not executed, alongside keeping the callable
  alive and a contrast with `Function`/`MoveOnlyFunction`, which own
  their callable and can't dangle this way
- `empty_function_call.cpp` — Calling an empty `Function`,
  `MoveOnlyFunction`, and `FunctionRef` alike, each throwing the same
  `std::bad_function_call`
- `use_after_move.cpp` — Calling a moved-from `Function` or
  `MoveOnlyFunction`, which throws rather than reading stale storage,
  contrasted with `FunctionRef`, whose trivial move never empties it
- `copying_move_only.cpp` — Attempting to copy a `MoveOnlyFunction`, a
  compile error shown but never compiled, alongside moving it, using
  `Function` instead, or using `FunctionRef` when ownership doesn't need
  to transfer at all

---

## Patterns

Demonstrates common usage idioms built on top of the core API —
deferred execution, task queues, key-based dispatch, and
observer-style callback subscription — using whichever of the three
types fits each idiom, with the mismatches called out explicitly.

### Examples

- `command_queue.cpp` — Queuing `MoveOnlyFunction` tasks (including one
  that owns a `std::unique_ptr`), a requeueable `Function`-based queue,
  and `FunctionRef` used for an immediate batch rather than a stored
  queue
- `deferred_execution.cpp` — Capturing a computation as a `Function` and
  running it later, `MoveOnlyFunction` deferring work that owns a
  resource, and `FunctionRef` deferring a call within the scope that
  declared it
- `visitor_dispatch.cpp` — A table of `FunctionRef` handlers dispatched
  by key, a `Function`-based table returned from a factory and able to
  outlive its builder, and a `MoveOnlyFunction`-based table of one-shot
  handlers
- `observer_callbacks.cpp` — A subject holding subscriber `Function`
  callbacks, a `MoveOnlyFunction`-based one-shot subscriber, and
  `FunctionRef` fanning an event out without a stored subject at all

---

## Quickstart

Demonstrates fundamental, everyday usage across all three types —
constructing, assigning, invoking, and checking whether a callable is
currently held or referenced.

### Examples

- `basic_usage.cpp` — Constructing and invoking `Function`,
  `MoveOnlyFunction`, and `FunctionRef` side by side, and resetting the
  two owning types
- `storing_lambdas.cpp` — Storing captureless and capturing lambdas, a
  function pointer, and a callable object in a `Function`; the same
  capture shapes in a `MoveOnlyFunction`; and referencing (not storing)
  a callable through a `FunctionRef`
- `checking_state.cpp` — Checking state via `operator bool()` and `==
  nullptr` on all three types, copying a `Function` versus the trivial
  copy of a `FunctionRef`, and swapping two `Function`s
