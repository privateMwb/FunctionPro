# Changelog

All notable changes to FunctionPro are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- ClusterFuzzLite fuzzing infrastructure: `fuzz_function`,
  `fuzz_move_only_function`, and `fuzz_function_ref` harnesses under
  `fuzz/`, PR-triggered and nightly batch fuzzing workflows, and
  `FUZZING.md` documenting what each harness covers.
- `static_assert` in `VTableFactory` requiring the bound callable type
  to be nothrow-move-constructible, since `VTable::move` and
  `VTable::destroy` are declared `noexcept` and rely on it — a
  violation now fails to compile with a clear message instead of
  risking `std::terminate()` at runtime.
- Repo banner, back-to-top badge, and divider SVG assets.

### Fixed
- `FunctionRef`'s object-storage constructor branch now preserves the
  cv-qualification of the referenced callable (invokes through the
  deduced type rather than the decayed type), so a `const` callable is
  only ever invoked through a `const`-qualified pointer.

### Changed
- Standardized the `@file`/`@date`/`@version`/`@copyright`/
  `@attention` header comment block across `Function.h`,
  `MoveOnlyFunction.h`, `FunctionRef.h`, and the `Detail/` headers
  (`CallableStorage.h`, `VTable.h`, `VTableFactory.h`, `SBOTraits.h`).
- Corrected the copyright holder from `MWB` to `privateMwb`.

## [1.0.0] - 2026-07-31

The first stable release of FunctionPro, a type-erased callable
wrapper library for modern C++.

### Added
- `Function<R(Args...)>` — copyable, `std::function`-like callable
  wrapper.
- `MoveOnlyFunction<R(Args...)>` — move-only wrapper for callables
  that can't be copied (e.g. one capturing a `std::unique_ptr`).
- `FunctionRef<R(Args...)>` — non-owning, zero-allocation reference to
  an existing callable.
- Small Buffer Optimization: small callables stored inline with zero
  heap allocation; larger callables fall back to the heap
  transparently, decided at compile time by `SBOTraits<T>::fits`.
- Strong exception guarantee on `Function`'s copy assignment — if the
  source's copy constructor throws, `*this` is left completely
  unaffected.
- `swap()` on `Function` and `MoveOnlyFunction` always goes through
  the callable's real move constructor, remaining correct even for
  callables with an internal self-pointer into their own storage.
- `operator bool()`, `== nullptr`, and `!= nullptr` for checking
  whether a callable is held or referenced.
- `reset()` on `Function` and `MoveOnlyFunction`.
- Empty-call safety: invoking any of the three types while empty
  throws `std::bad_function_call` rather than invoking undefined
  behavior.
- Type-erased dispatch through a single `VTable` pointer per instance,
  generated once per bound type by `VTableFactory`.
- `rain::` namespace alias for all three types, alongside their
  canonical `FunctionPro::` names.

### Performance
- Inline (SBO) storage avoids allocation wherever the callable's size
  and alignment allow it; heap fallback is transparent to the call
  site either way.
- `FunctionRef` never allocates and never copies the referenced
  callable, regardless of its size.
- `swap()` avoids a three-way temporary when only one side holds a
  callable, transferring it in a single move instead.
- Benchmarked against `std::function`, `std::move_only_function`, and
  `std::function_ref` at 10K / 100K / 1M iterations. Largest wins in
  moving and empty-call handling — heap-stored moves are a plain
  pointer transfer rather than a reallocation, and an empty call fails
  fast through a single vtable check. The two operations that
  deliberately cost more are `swap()` (always goes through the
  callable's real move constructor rather than a raw byte-swap, for
  self-pointer correctness) and `Copy-assign` (builds the copy in a
  temporary before touching `*this`, for the strong exception
  guarantee). Full results in `benchmarks/results/v1_0_0.md`.

### Testing
- Comprehensive test suite covering unit, integration, lifecycle, move
  semantics, copy semantics, regression, and exception-safety tests
  across all three types (`Function`, `MoveOnlyFunction`,
  `FunctionRef`); SBO/heap boundary behavior; empty-state and
  moved-from behavior; self-referencing callable correctness under
  `swap()`; and full construct/copy/move/invoke/destroy coverage of
  the type-erased dispatch layer (`VTableFactory`) across every
  distinct bound callable type introduced by the suite.
- 95.6% line coverage (152/159) and 99.1% function coverage
  (349/352), excluding test infrastructure and third-party
  dependencies. `FunctionPro/Detail` (`CallableStorage`, `SBOTraits`,
  `VTable`, `VTableFactory`) hits 100.0% line coverage.

### CI
- Automated builds and tests across GCC, Clang, MSVC, and AppleClang,
  each in Debug and Release configurations.

[Unreleased]: https://github.com/privateMwb/FunctionPro/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/privateMwb/FunctionPro/releases/tag/v1.0.0
