// FunctionPro Construct/Destroy Benchmark Suite
// Measures default construction and immediate destruction of an empty
// instance, for all three types against their std counterparts.
//
// An empty instance has no callable to store or size, so there's no
// small/large split here — that axis only exists once a callable is
// bound. Populated construction (small vs large capture) lives in
// core/bind.cpp; this file isolates the floor cost of an empty object
// with no vtable/callable set up at all.
//
// Covers:
// - Function default construct + destroy vs std::function
// - MoveOnlyFunction default construct + destroy vs
//   std::move_only_function, where available
// - FunctionRef default construct + destroy vs std::function_ref,
//   where available

#include <benchmark/benchmark.h>

#include <functional>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

// Measures default-constructing (and destroying) an empty Function.
static void Function_ConstructDestroy(benchmark::State& state) {
    for (auto _ : state) {
        Function<int()> f;
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(Function_ConstructDestroy);

// Measures default-constructing (and destroying) an empty
// std::function.
static void StdFunction_ConstructDestroy(benchmark::State& state) {
    for (auto _ : state) {
        std::function<int()> f;
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdFunction_ConstructDestroy);

// Measures default-constructing (and destroying) an empty
// MoveOnlyFunction.
static void MoveOnlyFunction_ConstructDestroy(benchmark::State& state) {
    for (auto _ : state) {
        MoveOnlyFunction<int()> f;
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(MoveOnlyFunction_ConstructDestroy);

#if defined(__cpp_lib_move_only_function)
// Measures default-constructing (and destroying) an empty
// std::move_only_function, where available.
static void StdMoveOnlyFunction_ConstructDestroy(benchmark::State& state) {
    for (auto _ : state) {
        std::move_only_function<int()> f;
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdMoveOnlyFunction_ConstructDestroy);
#endif

// Measures default-constructing (and destroying) an empty FunctionRef.
static void FunctionRef_ConstructDestroy(benchmark::State& state) {
    for (auto _ : state) {
        FunctionRef<int()> f;
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(FunctionRef_ConstructDestroy);

#if defined(__cpp_lib_function_ref)
// Measures default-constructing (and destroying) an empty
// std::function_ref, where available.
static void StdFunctionRef_ConstructDestroy(benchmark::State& state) {
    for (auto _ : state) {
        std::function_ref<int()> f;
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdFunctionRef_ConstructDestroy);
#endif
