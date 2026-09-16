// FunctionPro Bind Benchmark Suite
// Measures construction from a callable — the primary way a Function,
// MoveOnlyFunction, or FunctionRef comes into existence.
//
// Function and MoveOnlyFunction each get two cases: a small,
// SBO-fitting capturing lambda (SBO_SIZE = 40 bytes on a 64-bit build,
// stays inline, no allocation) and a large capturing lambda (forces a
// heap allocation) — these are genuinely different code paths with
// genuinely different costs. FunctionRef gets one case, not a
// small/large split: it never copies the callable, so its bind cost is
// O(1) regardless of size — a small/large split would just be the same
// instructions under two labels. It's kept in the file for contrast
// against the other two, since staying flat where they don't is itself
// the interesting result.
//
// Each timed iteration constructs (and immediately destroys) a fresh
// instance. The full size sweep across the SBO boundary lives in
// scaling/capture_size.cpp.
//
// Covers:
// - Function bind, small vs large capture
// - MoveOnlyFunction bind, small vs large capture, where available
// - FunctionRef bind, single case, where available

#include <benchmark/benchmark.h>

#include <array>
#include <functional>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

namespace {
constexpr int kPad = 3; // keeps the small lambda's capture well under 40 bytes

// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit.
struct LargePayload {
    std::array<std::byte, 64> padding{};
};
} // namespace

// Measures constructing (and destroying) a Function bound to a small,
// SBO-fitting capturing lambda.
static void Function_Bind_Small(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;

    for (auto _ : state) {
        Function<int()> f = [a, b, c] { return a + b + c; };
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(Function_Bind_Small);

// Measures constructing (and destroying) a std::function bound to a
// small, SBO-fitting capturing lambda.
static void StdFunction_Bind_Small(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;

    for (auto _ : state) {
        std::function<int()> f = [a, b, c] { return a + b + c; };
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdFunction_Bind_Small);

// Measures constructing (and destroying) a Function bound to a large,
// heap-forcing capturing lambda.
static void Function_Bind_Large(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        Function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(Function_Bind_Large);

// Measures constructing (and destroying) a std::function bound to a
// large, heap-forcing capturing lambda.
static void StdFunction_Bind_Large(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        std::function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdFunction_Bind_Large);

// Measures constructing (and destroying) a MoveOnlyFunction bound to a
// small, SBO-fitting capturing lambda.
static void MoveOnlyFunction_Bind_Small(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;

    for (auto _ : state) {
        MoveOnlyFunction<int()> f = [a, b, c] { return a + b + c; };
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(MoveOnlyFunction_Bind_Small);

#if defined(__cpp_lib_move_only_function)
// Measures constructing (and destroying) a std::move_only_function
// bound to a small, SBO-fitting capturing lambda, where available.
static void StdMoveOnlyFunction_Bind_Small(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;

    for (auto _ : state) {
        std::move_only_function<int()> f = [a, b, c] { return a + b + c; };
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdMoveOnlyFunction_Bind_Small);
#endif

// Measures constructing (and destroying) a MoveOnlyFunction bound to a
// large, heap-forcing capturing lambda.
static void MoveOnlyFunction_Bind_Large(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        MoveOnlyFunction<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(MoveOnlyFunction_Bind_Large);

#if defined(__cpp_lib_move_only_function)
// Measures constructing (and destroying) a std::move_only_function
// bound to a large, heap-forcing capturing lambda, where available.
static void StdMoveOnlyFunction_Bind_Large(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        std::move_only_function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdMoveOnlyFunction_Bind_Large);
#endif

// Measures constructing (and destroying) a FunctionRef bound to a
// capturing lambda. Not split by capture size — FunctionRef never
// copies the callable, so this cost is flat regardless of size.
static void FunctionRef_Bind(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;
    auto callable = [a, b, c] { return a + b + c; };

    for (auto _ : state) {
        FunctionRef<int()> f(callable);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(FunctionRef_Bind);

#if defined(__cpp_lib_function_ref)
// Measures constructing (and destroying) a std::function_ref bound to
// a capturing lambda, where available.
static void StdFunctionRef_Bind(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;
    auto callable = [a, b, c] { return a + b + c; };

    for (auto _ : state) {
        std::function_ref<int()> f(callable);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(StdFunctionRef_Bind);
#endif
