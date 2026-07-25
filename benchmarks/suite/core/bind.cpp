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
// Each timed call constructs (and immediately destroys) a fresh
// instance. The full size sweep across the SBO boundary lives in
// scaling/capture_size.cpp.
//
// Covers:
// - Function bind, small vs large capture
// - MoveOnlyFunction bind, small vs large capture, where available
// - FunctionRef bind, single case, where available

#include <support/framework.h>

#include <array>

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
static void bench_function_bind_small() {
    int a = kPad, b = kPad, c = kPad;

    auto cExpr = [&] {
        Function<int()> f = [a, b, c] { return a + b + c; };
        doNotOptimize(f);
    };

    auto sExpr = [&] {
        std::function<int()> f = [a, b, c] { return a + b + c; };
        doNotOptimize(f);
    };

    BENCH("Fn bind (small)", cExpr, sExpr);
}

// Measures constructing (and destroying) a Function bound to a large,
// heap-forcing capturing lambda.
static void bench_function_bind_large() {
    LargePayload payload{};

    auto cExpr = [&] {
        Function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        doNotOptimize(f);
    };

    auto sExpr = [&] {
        std::function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        doNotOptimize(f);
    };

    BENCH("Fn bind (large)", cExpr, sExpr);
}

// Measures constructing (and destroying) a MoveOnlyFunction bound to a
// small, SBO-fitting capturing lambda.
static void bench_move_only_bind_small() {
    int a = kPad, b = kPad, c = kPad;

    auto cExpr = [&] {
        MoveOnlyFunction<int()> f = [a, b, c] { return a + b + c; };
        doNotOptimize(f);
    };

#if defined(__cpp_lib_move_only_function)
    auto sExpr = [&] {
        std::move_only_function<int()> f = [a, b, c] { return a + b + c; };
        doNotOptimize(f);
    };
    BENCH("MoveOnlyFn bind (small)", cExpr, sExpr);
#else
    BENCH_SOLO("MoveOnlyFn bind (small)", cExpr);
#endif
}

// Measures constructing (and destroying) a MoveOnlyFunction bound to a
// large, heap-forcing capturing lambda.
static void bench_move_only_bind_large() {
    LargePayload payload{};

    auto cExpr = [&] {
        MoveOnlyFunction<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        doNotOptimize(f);
    };

#if defined(__cpp_lib_move_only_function)
    auto sExpr = [&] {
        std::move_only_function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        doNotOptimize(f);
    };
    BENCH("MoveOnlyFn bind (large)", cExpr, sExpr);
#else
    BENCH_SOLO("MoveOnlyFn bind (large)", cExpr);
#endif
}

// Measures constructing (and destroying) a FunctionRef bound to a
// capturing lambda. Not split by capture size — FunctionRef never
// copies the callable, so this cost is flat regardless of size.
static void bench_function_ref_bind() {
    int a = kPad, b = kPad, c = kPad;
    auto callable = [a, b, c] { return a + b + c; };

    auto cExpr = [&] {
        FunctionRef<int()> f(callable);
        doNotOptimize(f);
    };

#if defined(__cpp_lib_function_ref)
    auto sExpr = [&] {
        std::function_ref<int()> f(callable);
        doNotOptimize(f);
    };
    BENCH("FnRef bind", cExpr, sExpr);
#else
    BENCH_SOLO("FnRef bind", cExpr);
#endif
}

// Executes all bind benchmark cases.
static void run_benchmarks() {
    bench_function_bind_small();
    std::cout << "\n";

    bench_function_bind_large();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::move_only_function");
    setHeader(suiteName);

    bench_move_only_bind_small();
    std::cout << "\n";

    bench_move_only_bind_large();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::function_ref");
    setHeader(suiteName);

    bench_function_ref_bind();
}

REGISTER_BENCH_SUITE();
