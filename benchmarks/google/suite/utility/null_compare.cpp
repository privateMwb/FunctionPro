// FunctionPro Null Compare Benchmark Suite
// Measures operator==(nullptr) performance for Function, MoveOnlyFunction,
// and FunctionRef against their std counterparts, on a bound (non-null)
// instance. Not split empty vs bound — same reasoning as
// access/bool_check.cpp: it's the same null-pointer check either way,
// not a genuinely different cost.
//
// FunctionRef has no std counterpart here at all, not just a
// toolchain-gated one: std::function_ref (P0792) is designed to be
// non-nullable — no default constructor, always bound to a valid
// callable at construction — so it has no operator==(nullptr_t)
// equivalent, regardless of toolchain version. Our FunctionRef
// explicitly supports an empty, default-constructed state, which is a
// real design difference worth noting rather than papering over with a
// feature-test gate.
//
// Covers:
// - Function::operator==(nullptr) vs std::function
// - MoveOnlyFunction::operator==(nullptr) vs std::move_only_function,
//   where available
// - FunctionRef::operator==(nullptr), solo — no std::function_ref
//   equivalent exists

#include <benchmark/benchmark.h>

#include <functional>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

// Measures Function::operator==(nullptr).
static void Function_NullCompare(benchmark::State& state) {
    Function<int()> cSrc = [] { return 1; };

    for (auto _ : state) {
        bool v = (cSrc == nullptr);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(Function_NullCompare);

// Measures std::function::operator==(nullptr).
static void StdFunction_NullCompare(benchmark::State& state) {
    std::function<int()> sSrc = [] { return 1; };

    for (auto _ : state) {
        bool v = (sSrc == nullptr);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(StdFunction_NullCompare);

// Measures MoveOnlyFunction::operator==(nullptr).
static void MoveOnlyFunction_NullCompare(benchmark::State& state) {
    MoveOnlyFunction<int()> cSrc = [] { return 1; };

    for (auto _ : state) {
        bool v = (cSrc == nullptr);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(MoveOnlyFunction_NullCompare);

#if defined(__cpp_lib_move_only_function)
// Measures std::move_only_function::operator==(nullptr), where
// available.
static void StdMoveOnlyFunction_NullCompare(benchmark::State& state) {
    std::move_only_function<int()> sSrc = [] { return 1; };

    for (auto _ : state) {
        bool v = (sSrc == nullptr);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(StdMoveOnlyFunction_NullCompare);
#endif

// Measures FunctionRef::operator==(nullptr), solo — std::function_ref
// has no equivalent (non-nullable by design, see file header).
static void FunctionRef_NullCompare(benchmark::State& state) {
    auto callable = [] { return 1; };
    FunctionRef<int()> cSrc(callable);

    for (auto _ : state) {
        bool v = (cSrc == nullptr);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(FunctionRef_NullCompare);
