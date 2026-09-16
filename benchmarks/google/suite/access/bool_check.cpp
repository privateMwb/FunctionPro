// FunctionPro Bool Check Benchmark Suite
// Measures operator bool() performance for Function, MoveOnlyFunction,
// and FunctionRef against their std counterparts, on a bound (non-null)
// instance.
//
// Covers:
// - Function::operator bool() vs std::function::operator bool()
// - MoveOnlyFunction::operator bool() vs
//   std::move_only_function::operator bool() (guarded by
//   __cpp_lib_move_only_function)
// - FunctionRef::operator bool() vs std::function_ref::operator bool()
//   (guarded by __cpp_lib_function_ref)

#include <benchmark/benchmark.h>

#include <functional>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

// Measures Function::operator bool().
static void Function_Bool(benchmark::State& state) {
    Function<int()> cSrc = [] { return 1; };

    for (auto _ : state) {
        bool v = static_cast<bool>(cSrc);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(Function_Bool);

// Measures std::function::operator bool().
static void StdFunction_Bool(benchmark::State& state) {
    std::function<int()> sSrc = [] { return 1; };

    for (auto _ : state) {
        bool v = static_cast<bool>(sSrc);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(StdFunction_Bool);

// Measures MoveOnlyFunction::operator bool().
static void MoveOnlyFunction_Bool(benchmark::State& state) {
    MoveOnlyFunction<int()> cSrc = [] { return 1; };

    for (auto _ : state) {
        bool v = static_cast<bool>(cSrc);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(MoveOnlyFunction_Bool);

#if defined(__cpp_lib_move_only_function)
// Measures std::move_only_function::operator bool(), where available.
static void StdMoveOnlyFunction_Bool(benchmark::State& state) {
    std::move_only_function<int()> sSrc = [] { return 1; };

    for (auto _ : state) {
        bool v = static_cast<bool>(sSrc);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(StdMoveOnlyFunction_Bool);
#endif

// Measures FunctionRef::operator bool().
static void FunctionRef_Bool(benchmark::State& state) {
    auto callable = [] { return 1; };
    FunctionRef<int()> cSrc(callable);

    for (auto _ : state) {
        bool v = static_cast<bool>(cSrc);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(FunctionRef_Bool);

#if defined(__cpp_lib_function_ref)
// Measures std::function_ref::operator bool(), where available.
static void StdFunctionRef_Bool(benchmark::State& state) {
    auto callable = [] { return 1; };
    std::function_ref<int()> sSrc(callable);

    for (auto _ : state) {
        bool v = static_cast<bool>(sSrc);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(StdFunctionRef_Bool);
#endif
