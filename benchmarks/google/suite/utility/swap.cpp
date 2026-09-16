// FunctionPro Swap Benchmark Suite
// Measures swap() performance for Function and MoveOnlyFunction against
// their std counterparts. FunctionRef has no swap() member at all — not
// even a solo case, the operation simply doesn't exist on it — so it's
// excluded entirely.
//
// swap() deliberately goes through the vtable's move operation on both
// sides rather than exchanging raw storage bytes, so this is a real,
// measurable cost distinct from a hypothetical byte-swap — not the same
// benchmark as move-assign (core/assign.cpp) under a different name,
// since swap does two moves in sequence rather than one.
//
// Covers:
// - Function::swap() vs std::function::swap()
// - MoveOnlyFunction::swap() vs std::move_only_function::swap(), where
//   available

#include <benchmark/benchmark.h>

#include <functional>

#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

// Measures Function::swap().
static void Function_Swap(benchmark::State& state) {
    Function<int()> cA = [] { return 1; };
    Function<int()> cB = [] { return 2; };

    for (auto _ : state) {
        cA.swap(cB);
    }
}
BENCHMARK(Function_Swap);

// Measures std::function::swap().
static void StdFunction_Swap(benchmark::State& state) {
    std::function<int()> sA = [] { return 1; };
    std::function<int()> sB = [] { return 2; };

    for (auto _ : state) {
        sA.swap(sB);
    }
}
BENCHMARK(StdFunction_Swap);

// Measures MoveOnlyFunction::swap().
static void MoveOnlyFunction_Swap(benchmark::State& state) {
    MoveOnlyFunction<int()> cA = [] { return 1; };
    MoveOnlyFunction<int()> cB = [] { return 2; };

    for (auto _ : state) {
        cA.swap(cB);
    }
}
BENCHMARK(MoveOnlyFunction_Swap);

#if defined(__cpp_lib_move_only_function)
// Measures std::move_only_function::swap(), where available.
static void StdMoveOnlyFunction_Swap(benchmark::State& state) {
    std::move_only_function<int()> sA = [] { return 1; };
    std::move_only_function<int()> sB = [] { return 2; };

    for (auto _ : state) {
        sA.swap(sB);
    }
}
BENCHMARK(StdMoveOnlyFunction_Swap);
#endif
