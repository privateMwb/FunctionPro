// FunctionPro Move Semantics Benchmark Suite
// Measures move construction. Move-assign already lives in
// core/assign.cpp, so this file covers move-construct only.
//
// Function and MoveOnlyFunction both split small vs large capture — and
// this is the interesting split: an SBO-stored callable has to
// move-construct T into the new inline buffer (real work), while a
// heap-stored callable just swaps a pointer (O(1), no touching T at
// all). Expect heap move to come out faster, not slower — the opposite
// of the pattern in copy_semantics.cpp. FunctionRef stays a single
// case: trivially-copyable pointer pair, flat regardless of size.
//
// Each timed iteration rebuilds the source, since a moved-from
// instance is left empty.
//
// Covers:
// - Function move construct, small vs large capture, vs std::function
// - MoveOnlyFunction move construct, small vs large capture, vs
//   std::move_only_function, where available
// - FunctionRef move construct vs std::function_ref, where available

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

// Measures Function move construct, small (SBO-fitting) capture.
static void Function_MoveCtor_Small(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;

    for (auto _ : state) {
        Function<int()> src = [a, b, c] { return a + b + c; };
        Function<int()> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(Function_MoveCtor_Small);

// Measures std::function move construct, small (SBO-fitting) capture.
static void StdFunction_MoveCtor_Small(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;

    for (auto _ : state) {
        std::function<int()> src = [a, b, c] { return a + b + c; };
        std::function<int()> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(StdFunction_MoveCtor_Small);

// Measures Function move construct, large (heap-forcing) capture.
static void Function_MoveCtor_Large(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        Function<int()> src = [payload] {
            (void)payload;
            return 1;
        };
        Function<int()> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(Function_MoveCtor_Large);

// Measures std::function move construct, large (heap-forcing) capture.
static void StdFunction_MoveCtor_Large(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        std::function<int()> src = [payload] {
            (void)payload;
            return 1;
        };
        std::function<int()> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(StdFunction_MoveCtor_Large);

// Measures MoveOnlyFunction move construct, small (SBO-fitting)
// capture.
static void MoveOnlyFunction_MoveCtor_Small(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;

    for (auto _ : state) {
        MoveOnlyFunction<int()> src = [a, b, c] { return a + b + c; };
        MoveOnlyFunction<int()> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(MoveOnlyFunction_MoveCtor_Small);

#if defined(__cpp_lib_move_only_function)
// Measures std::move_only_function move construct, small
// (SBO-fitting) capture, where available.
static void StdMoveOnlyFunction_MoveCtor_Small(benchmark::State& state) {
    int a = kPad, b = kPad, c = kPad;

    for (auto _ : state) {
        std::move_only_function<int()> src = [a, b, c] { return a + b + c; };
        std::move_only_function<int()> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(StdMoveOnlyFunction_MoveCtor_Small);
#endif

// Measures MoveOnlyFunction move construct, large (heap-forcing)
// capture.
static void MoveOnlyFunction_MoveCtor_Large(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        MoveOnlyFunction<int()> src = [payload] {
            (void)payload;
            return 1;
        };
        MoveOnlyFunction<int()> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(MoveOnlyFunction_MoveCtor_Large);

#if defined(__cpp_lib_move_only_function)
// Measures std::move_only_function move construct, large
// (heap-forcing) capture, where available.
static void StdMoveOnlyFunction_MoveCtor_Large(benchmark::State& state) {
    LargePayload payload{};

    for (auto _ : state) {
        std::move_only_function<int()> src = [payload] {
            (void)payload;
            return 1;
        };
        std::move_only_function<int()> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(StdMoveOnlyFunction_MoveCtor_Large);
#endif

// Measures FunctionRef move construct. Not split by capture size —
// trivially-copyable pointer pair, flat regardless of the referenced
// callable.
static void FunctionRef_MoveCtor(benchmark::State& state) {
    auto callable = [] { return 1; };

    for (auto _ : state) {
        FunctionRef<int()> src(callable);
        FunctionRef<int()> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(FunctionRef_MoveCtor);

#if defined(__cpp_lib_function_ref)
// Measures std::function_ref move construct, where available.
static void StdFunctionRef_MoveCtor(benchmark::State& state) {
    auto callable = [] { return 1; };

    for (auto _ : state) {
        std::function_ref<int()> src(callable);
        std::function_ref<int()> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(StdFunctionRef_MoveCtor);
#endif
