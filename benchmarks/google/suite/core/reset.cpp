// FunctionPro Reset Benchmark Suite
// Measures reset() performance for Function and MoveOnlyFunction against
// their std counterparts. FunctionRef has no reset() — it's a trivial,
// non-owning view with no lifetime to release — so it's excluded here.
//
// Each case starts from a heap-allocated (large-capture) bound instance
// so reset() has real teardown work to do, not just a null-check.
// std::function/std::move_only_function have no reset() member; the
// idiomatic equivalent is assignment from nullptr, used here as their
// side of the comparison.
//
// Covers:
// - Function::reset() vs std::function `f = nullptr`
// - MoveOnlyFunction::reset() vs std::move_only_function `f = nullptr`,
//   where available (guarded by __cpp_lib_move_only_function)

#include <benchmark/benchmark.h>

#include <array>
#include <functional>

#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

namespace {
// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit,
// so reset() has an actual heap deallocation to perform each call.
struct LargePayload {
    std::array<std::byte, 64> padding{};
};
} // namespace

// Measures Function::reset().
static void Function_Reset(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        Function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        f.reset();
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(Function_Reset);

// Measures std::function's `f = nullptr` (the idiomatic reset
// equivalent).
static void StdFunction_Reset(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        std::function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        f = nullptr;
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdFunction_Reset);

// Measures MoveOnlyFunction::reset().
static void MoveOnlyFunction_Reset(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        MoveOnlyFunction<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        f.reset();
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(MoveOnlyFunction_Reset);

#if defined(__cpp_lib_move_only_function)
// Measures std::move_only_function's `f = nullptr` (the idiomatic
// reset equivalent), where available.
static void StdMoveOnlyFunction_Reset(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        std::move_only_function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        f = nullptr;
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdMoveOnlyFunction_Reset);
#endif
