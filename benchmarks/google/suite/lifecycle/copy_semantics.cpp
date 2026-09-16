// FunctionPro Copy Semantics Benchmark Suite
// Measures copy construction. MoveOnlyFunction is excluded entirely —
// its copy constructor is deleted (matching std::move_only_function),
// there's nothing to benchmark.
//
// Function splits small vs large capture — genuinely different costs:
// an SBO-stored callable copies by move/copy-constructing T directly
// into the new inline buffer, while a heap-stored callable copies by
// allocating new heap storage and copying T into it. FunctionRef stays
// a single case: it's a trivially-copyable pointer pair, so its copy
// cost is flat regardless of the referenced callable's size.
//
// Covers:
// - Function copy construct, small vs large capture, vs std::function
// - FunctionRef copy construct vs std::function_ref, where available

#include <benchmark/benchmark.h>

#include <array>
#include <functional>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>

using namespace FunctionPro;

namespace {
constexpr int kPad = 3; // keeps the small lambda's capture well under 40 bytes

// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit.
struct LargePayload {
    std::array<std::byte, 64> padding{};
};
} // namespace

// Measures Function copy construct, small (SBO-fitting) capture.
static void Function_CopyCtor_Small(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;
    Function<int()> src = [a, b, c] { return a + b + c; };

    for (auto _ : state) {
        Function<int()> f(src);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(Function_CopyCtor_Small);

// Measures std::function copy construct, small (SBO-fitting) capture.
static void StdFunction_CopyCtor_Small(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;
    std::function<int()> src = [a, b, c] { return a + b + c; };

    for (auto _ : state) {
        std::function<int()> f(src);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdFunction_CopyCtor_Small);

// Measures Function copy construct, large (heap-forcing) capture.
static void Function_CopyCtor_Large(benchmark::State& state) {
    LargePayload payload{};
    Function<int()> src = [payload] {
        (void)payload;
        return 1;
    };

    for (auto _ : state) {
        Function<int()> f(src);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(Function_CopyCtor_Large);

// Measures std::function copy construct, large (heap-forcing) capture.
static void StdFunction_CopyCtor_Large(benchmark::State& state) {
    LargePayload payload{};
    std::function<int()> src = [payload] {
        (void)payload;
        return 1;
    };

    for (auto _ : state) {
        std::function<int()> f(src);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdFunction_CopyCtor_Large);

// Measures FunctionRef copy construct. Not split by capture size — the
// copy is a flat, trivial pointer-pair copy regardless of the
// referenced callable.
static void FunctionRef_CopyCtor(benchmark::State& state) {
    auto callable = [] { return 1; };
    FunctionRef<int()> src(callable);

    for (auto _ : state) {
        FunctionRef<int()> f(src);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(FunctionRef_CopyCtor);

#if defined(__cpp_lib_function_ref)
// Measures std::function_ref copy construct, where available.
static void StdFunctionRef_CopyCtor(benchmark::State& state) {
    auto callable = [] { return 1; };
    std::function_ref<int()> src(callable);

    for (auto _ : state) {
        std::function_ref<int()> f(src);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdFunctionRef_CopyCtor);
#endif
