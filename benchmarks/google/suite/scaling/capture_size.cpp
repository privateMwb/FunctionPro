// FunctionPro Capture Size Benchmark Suite
// Measures construction cost against std::function as capture-state
// size grows, crossing the SBO_SIZE = 40 byte boundary (64-bit build)
// that decides whether a callable stays inline or moves to the heap.
//
// Function only. MoveOnlyFunction shares the exact same
// CallableStorage/SBOTraits/VTableFactory backend for this dispatch —
// the same if-constexpr machinery either way — so sweeping it too would
// just show the same shape under a different label. FunctionRef has no
// such axis at all: it never copies the callable, so its cost is flat
// regardless of size (see core/bind.cpp).
//
// Five points: 0 bytes (captureless), 16 bytes (well inside SBO),
// 40 bytes (exactly at the boundary), 64 bytes (just past it), and
// 256 bytes (well into heap territory). Expect a step up somewhere
// between 40 and 64 bytes as construction starts allocating. Invoke
// cost across this same sweep — expected to stay flat instead — is
// covered separately in invoke_by_size.cpp.
//
// Covers:
// - Function construction cost at 0B, 16B, 40B, 64B, 256B capture

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

// Measures constructing (and destroying) a Function bound to a
// callable of a given capture size N.
template <std::size_t N> static void Function_Bind_BySize(benchmark::State& state) {
    SizedCallable<N> callable{};

    for (auto _ : state) {
        Function<int()> f(callable);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK_TEMPLATE(Function_Bind_BySize, 0);
BENCHMARK_TEMPLATE(Function_Bind_BySize, 16);
BENCHMARK_TEMPLATE(Function_Bind_BySize, 40);
BENCHMARK_TEMPLATE(Function_Bind_BySize, 64);
BENCHMARK_TEMPLATE(Function_Bind_BySize, 256);

// Measures constructing (and destroying) a std::function bound to a
// callable of a given capture size N.
template <std::size_t N> static void StdFunction_Bind_BySize(benchmark::State& state) {
    SizedCallable<N> callable{};

    for (auto _ : state) {
        std::function<int()> f(callable);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK_TEMPLATE(StdFunction_Bind_BySize, 0);
BENCHMARK_TEMPLATE(StdFunction_Bind_BySize, 16);
BENCHMARK_TEMPLATE(StdFunction_Bind_BySize, 40);
BENCHMARK_TEMPLATE(StdFunction_Bind_BySize, 64);
BENCHMARK_TEMPLATE(StdFunction_Bind_BySize, 256);
