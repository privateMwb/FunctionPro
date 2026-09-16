// FunctionPro Invoke By Size Benchmark Suite
// Measures operator() cost against std::function's across the same
// capture-size sweep as capture_size.cpp — same five points, same
// SBO_SIZE = 40 byte boundary — but timing invocation instead of
// construction.
//
// Function only, for the same reason as capture_size.cpp:
// MoveOnlyFunction shares the identical dispatch backend, and
// FunctionRef never copies the callable in the first place.
//
// Unlike construction, invocation goes through a vtable indirect call
// regardless of whether the callable lives inline or on the heap, so
// the expected (and worth demonstrating) result here is a flat line —
// the opposite shape from construction's step at the SBO boundary.
//
// Covers:
// - Function invoke cost at 0B, 16B, 40B, 64B, 256B capture

#include <benchmark/benchmark.h>

#include <array>
#include <functional>

#include <FunctionPro/Function.h>

using namespace FunctionPro;

namespace {
template <std::size_t N> struct SizedCallable {
    std::array<std::byte, N> padding{};
    int operator()() const {
        (void)padding;
        return 1;
    }
};
} // namespace

// Measures invoking a Function bound to a callable of a given capture
// size N, hit path.
template <std::size_t N> static void Function_Invoke_BySize(benchmark::State& state) {
    SizedCallable<N> callable{};
    Function<int()> cSrc(callable);

    for (auto _ : state) {
        int v = cSrc();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK_TEMPLATE(Function_Invoke_BySize, 0);
BENCHMARK_TEMPLATE(Function_Invoke_BySize, 16);
BENCHMARK_TEMPLATE(Function_Invoke_BySize, 40);
BENCHMARK_TEMPLATE(Function_Invoke_BySize, 64);
BENCHMARK_TEMPLATE(Function_Invoke_BySize, 256);

// Measures invoking a std::function bound to a callable of a given
// capture size N, hit path.
template <std::size_t N> static void StdFunction_Invoke_BySize(benchmark::State& state) {
    SizedCallable<N> callable{};
    std::function<int()> sSrc(callable);

    for (auto _ : state) {
        int v = sSrc();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK_TEMPLATE(StdFunction_Invoke_BySize, 0);
BENCHMARK_TEMPLATE(StdFunction_Invoke_BySize, 16);
BENCHMARK_TEMPLATE(StdFunction_Invoke_BySize, 40);
BENCHMARK_TEMPLATE(StdFunction_Invoke_BySize, 64);
BENCHMARK_TEMPLATE(StdFunction_Invoke_BySize, 256);
