// FunctionPro Assign Benchmark Suite
// Measures copy-assign and move-assign performance across Function,
// MoveOnlyFunction, and FunctionRef against their std counterparts.
//
// Function gets both a copy-assign and a move-assign case — these are
// genuinely different code paths (copy-assign goes through a
// copy-construct-then-swap for the strong exception guarantee;
// move-assign goes through the vtable's move operation directly).
// MoveOnlyFunction gets move-assign only — copy-assign is deleted, it
// doesn't exist. FunctionRef gets a single "assign" case, not split
// into copy/move variants: it's a trivially-copyable pointer pair, so
// its copy-assign and move-assign compile to the identical instruction
// sequence — labeling both would be the same benchmark twice.
//
// Move-assign cases ping-pong a populated instance between two slots
// (a -> b, then b -> a, ...) so no per-call rebuild cost leaks into the
// measurement — only the move itself is timed.
//
// Covers:
// - Function copy-assign vs std::function
// - Function move-assign vs std::function
// - MoveOnlyFunction move-assign vs std::move_only_function, where available
// - FunctionRef assign vs std::function_ref, where available

#include <benchmark/benchmark.h>

#include <functional>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

// Measures Function copy-assign.
static void Function_CopyAssign(benchmark::State& state) {
    Function<int()> cSrc = [] { return 1; };
    Function<int()> cDst;

    for (auto _ : state) {
        cDst = cSrc;
    }
}
BENCHMARK(Function_CopyAssign);

// Measures std::function copy-assign.
static void StdFunction_CopyAssign(benchmark::State& state) {
    std::function<int()> sSrc = [] { return 1; };
    std::function<int()> sDst;

    for (auto _ : state) {
        sDst = sSrc;
    }
}
BENCHMARK(StdFunction_CopyAssign);

// Measures Function move-assign, ping-ponging between two slots so no
// per-iteration rebuild cost leaks into the measurement.
static void Function_MoveAssign(benchmark::State& state) {
    Function<int()> cA = [] { return 1; };
    Function<int()> cB;
    bool flip = false;

    for (auto _ : state) {
        if (!flip)
            cB = std::move(cA);
        else
            cA = std::move(cB);
        flip = !flip;
    }
}
BENCHMARK(Function_MoveAssign);

// Measures std::function move-assign, ping-ponging between two slots.
static void StdFunction_MoveAssign(benchmark::State& state) {
    std::function<int()> sA = [] { return 1; };
    std::function<int()> sB;
    bool flip = false;

    for (auto _ : state) {
        if (!flip)
            sB = std::move(sA);
        else
            sA = std::move(sB);
        flip = !flip;
    }
}
BENCHMARK(StdFunction_MoveAssign);

// Measures MoveOnlyFunction move-assign, ping-ponging between two
// slots.
static void MoveOnlyFunction_MoveAssign(benchmark::State& state) {
    MoveOnlyFunction<int()> cA = [] { return 1; };
    MoveOnlyFunction<int()> cB;
    bool flip = false;

    for (auto _ : state) {
        if (!flip)
            cB = std::move(cA);
        else
            cA = std::move(cB);
        flip = !flip;
    }
}
BENCHMARK(MoveOnlyFunction_MoveAssign);

#if defined(__cpp_lib_move_only_function)
// Measures std::move_only_function move-assign, ping-ponging between
// two slots, where available.
static void StdMoveOnlyFunction_MoveAssign(benchmark::State& state) {
    std::move_only_function<int()> sA = [] { return 1; };
    std::move_only_function<int()> sB;
    bool flip = false;

    for (auto _ : state) {
        if (!flip)
            sB = std::move(sA);
        else
            sA = std::move(sB);
        flip = !flip;
    }
}
BENCHMARK(StdMoveOnlyFunction_MoveAssign);
#endif

// Measures FunctionRef's trivial assign. Not split into copy/move —
// both compile to the same instruction sequence for a
// trivially-copyable pointer pair.
static void FunctionRef_Assign(benchmark::State& state) {
    auto callable = [] { return 1; };
    FunctionRef<int()> cSrc(callable);
    FunctionRef<int()> cDst;

    for (auto _ : state) {
        cDst = cSrc;
    }
}
BENCHMARK(FunctionRef_Assign);

#if defined(__cpp_lib_function_ref)
// Measures std::function_ref's trivial assign, where available.
static void StdFunctionRef_Assign(benchmark::State& state) {
    auto callable = [] { return 1; };
    std::function_ref<int()> sSrc(callable);
    std::function_ref<int()> sDst = sSrc;

    for (auto _ : state) {
        sDst = sSrc;
    }
}
BENCHMARK(StdFunctionRef_Assign);
#endif
