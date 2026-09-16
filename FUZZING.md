# Fuzzing

FunctionPro is fuzzed via [ClusterFuzzLite](https://google.github.io/clusterfuzzlite/),
running on every pull request that touches the fuzzed files, plus a
longer scheduled batch run every night.

## What's covered

### `fuzz_function.cpp` — `Function<int(int)>`

A differential fuzzer: it runs the same sequence of operations against
`FunctionPro::Function<int(int)>` and a `std::function<int(int)>`
reference, comparing emptiness and invocation results after every
single call (not just at the end), so a failing input localizes to the
exact operation that broke an invariant.

Both containers wrap the *same* callable objects — a small,
SBO-eligible struct and a deliberately oversized one that forces heap
storage — rather than the harness reimplementing the expected
behavior separately. Both callables mutate an internal counter on
every call through a non-const `operator()`, which specifically
exercises `Function::operator()` invoking a mutable callable through
its own `const` member function (the `const_cast` internal to
`VTableFactory::invoke`) — exactly where a subtle aliasing or
lifetime bug in the type-erasure layer would hide.

Specifically exercised:

- **SBO vs. heap invocation correctness**, including repeated
  reassignment between the small (inline) and large (heap-allocated)
  callable, which forces `Function` through the storage-transition
  path on every such op rather than settling into one representation.
- **Copy-assign and move-assign**, including cross-slot assignment
  between two independently tracked `Function`/`std::function` pairs
  (not just assigning a fresh value each time).
- **`swap()`**, including a same-callable cross-slot swap, exercising
  the triangular move-through-temporary swap implementation.
- **Self-copy-assign and self-move-assign safety.** Checked as a pure
  `Function`-side invariant — `bool()` must be unchanged by a
  self-assignment — rather than differential-tested against
  `std::function`, since the standard doesn't guarantee
  `std::function`'s self-move-assignment behavior portably enough to
  rely on for a cross-implementation comparison.
- **Self-swap safety**, same reasoning as self-move-assign.
- **`reset()`** and the invariant that `bool(fn)` and
  `fn == nullptr` must always disagree with each other — checked
  after every single operation in the harness, not just the ones that
  explicitly target it.
- **Invoking an empty `Function` throws `std::bad_function_call`** at
  exactly the same points `std::function` would.

### `fuzz_move_only_function.cpp` — `MoveOnlyFunction<int(int)>`

Can't differential-test against a `std::function`-like reference:
`MoveOnlyFunction` wraps genuinely move-only callables, and
`std::move_only_function` requires C++23 (this project targets
C++20). Instead, each callable's internal state (a single `int` behind
a `std::unique_ptr`, which is what makes the callable itself
non-copyable) is tracked by the harness as ground truth — the
callables' `operator()` always does exactly `*state += 1; return
*state + x;`, a simple enough contract that the harness can predict
the exact next result without needing a second implementation to
compare against.

Specifically exercised: the same list as `fuzz_function.cpp` above
(SBO/heap transitions, cross-slot move-assign and swap, self-move-
assign/self-swap safety, `reset()`, the `bool()`/`==nullptr`
invariant, empty-call `std::bad_function_call`), plus one guarantee
`Function` doesn't make: **the moved-from side of a move-assign or
swap is verified to be left empty** — a hard contract for
`MoveOnlyFunction`, unlike `std::function`'s "valid but unspecified"
moved-from state.

Deliberately not covered: exception injection. `MoveOnlyFunction` has
no copy path at all, so there's nothing analogous to `Function`'s
strong-exception-guarantee copy-assign to stress here.

### `fuzz_function_ref.cpp` — `FunctionRef<int(int)>`

`FunctionRef` is non-owning and trivially copyable — there's no
container lifecycle (no copy/move of *contents*, no heap allocation)
to fuzz the way the other two harnesses fuzz storage. Instead this
repeatedly rebinds a single `FunctionRef` to a fixed set of referents
that all outlive the whole run, and after each invocation reads the
*real* referenced object's own state directly to confirm the call
actually reached the correct object with the correct argument, rather
than trusting `FunctionRef`'s own return value in isolation (which
would just be checking the implementation against itself).

Specifically exercised:

- Binding to a mutable functor object (non-`const` `operator()`).
- Binding to a `const`-qualified functor object whose `operator()` is
  `const` — exercising the cv-qualification-preserving invoke path
  (`StoredT = T`, not the decayed type), so a `const` referent is only
  ever invoked through a `const`-qualified pointer.
- Binding to a raw function reference (decays to a pointer inside the
  constructor) and, separately, to an already-a-function-pointer
  variable — the two distinct branches in `FunctionRef`'s constructor
  for free functions.
- Rebinding to empty and confirming invoking an empty `FunctionRef`
  throws `std::bad_function_call`.
- Copying a `FunctionRef` value and invoking through the copy,
  confirming it reaches the same underlying object as the original.
- The `bool()`/`operator==(nullptr)` invariant, checked after every
  single operation.

Deliberately not covered: referent lifetime violations (invoking a
`FunctionRef` after its referent has been destroyed). That's
documented, contractual UB by design — "the callable must outlive the
`FunctionRef` instance" — not a bug for this harness to try to
trigger; every referent in the harness is a stack local that outlives
the whole fuzz iteration.

All three harnesses are built and run under both AddressSanitizer and
UndefinedBehaviorSanitizer.

## What's deliberately NOT covered yet

- **Exception injection during `Function`'s copy construction/
  assignment.** `fuzz_function.cpp` never makes a stored callable's
  copy constructor throw, so it never exercises `Function`'s
  strong-exception-guarantee rollback path — that's covered
  separately by the `throwing_copy_constructor` unit test suite, not
  by fuzzing.
- **`FunctionRef` referent lifetime violations** and
  **`MoveOnlyFunction`/`Function` exception injection for
  `MoveOnlyFunction`** — see the per-harness notes above; both are
  either contractual UB by design or simply don't apply to that type.

## Running locally

```bash
git clone --recursive https://github.com/google/oss-fuzz.git
cd oss-fuzz
python infra/helper.py build_fuzzers --sanitizer address FunctionPro /path/to/FunctionPro
python infra/helper.py run_fuzzer FunctionPro fuzz_function
python infra/helper.py run_fuzzer FunctionPro fuzz_move_only_function
python infra/helper.py run_fuzzer FunctionPro fuzz_function_ref
```

Or, without OSS-Fuzz's tooling, directly with clang (substitute
`fuzz_function` for whichever harness you want):

```bash
clang++ -std=c++20 -fsanitize=fuzzer,address \
  -Iinclude \
  fuzz/fuzz_function.cpp \
  -o fuzz_function

./fuzz_function
```

Add `-fsanitize=fuzzer,undefined` instead to run under UBSan.

## Reproducing a crash

ClusterFuzzLite uploads the failing input as a workflow artifact when
a run fails, named after whichever harness found it. Download it, then:

```bash
./fuzz_function path/to/crash-<hash>
```

This replays that exact byte sequence through
`LLVMFuzzerTestOneInput()` once, deterministically — no sanitizer flags
needed beyond however the binary was already built.

## Adding a new harness

1. Add `fuzz/fuzz_<target>.cpp` with an `extern "C" int
   LLVMFuzzerTestOneInput(const uint8_t*, size_t)` entry point.
2. Add the matching compile + link block to `.clusterfuzzlite/build.sh`.
3. No workflow changes needed — `cflite_pr.yml`/`cflite_batch.yml`
   build and run every binary `build.sh` produces in `$OUT`.
