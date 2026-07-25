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
// Covers:
// - Function move construct, small vs large capture, vs std::function
// - MoveOnlyFunction move construct, small vs large capture, vs
//   std::move_only_function, where available
// - FunctionRef move construct vs std::function_ref, where available

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

// Measures Function move construct against std::function's, small
// (SBO-fitting) capture. Rebuilds the source each call since a
// moved-from instance is left empty.
static void bench_function_move_small() {
    int a = kPad, b = kPad, c = kPad;

    auto cExpr = [&] {
        Function<int()> src = [a, b, c] { return a + b + c; };
        Function<int()> dst(std::move(src));
        doNotOptimize(dst);
    };

    auto sExpr = [&] {
        std::function<int()> src = [a, b, c] { return a + b + c; };
        std::function<int()> dst(std::move(src));
        doNotOptimize(dst);
    };

    BENCH("Fn move construct (small)", cExpr, sExpr);
}

// Measures Function move construct against std::function's, large
// (heap-forcing) capture.
static void bench_function_move_large() {
    LargePayload payload{};

    auto cExpr = [&] {
        Function<int()> src = [payload] {
            (void)payload;
            return 1;
        };
        Function<int()> dst(std::move(src));
        doNotOptimize(dst);
    };

    auto sExpr = [&] {
        std::function<int()> src = [payload] {
            (void)payload;
            return 1;
        };
        std::function<int()> dst(std::move(src));
        doNotOptimize(dst);
    };

    BENCH("Fn move construct (large)", cExpr, sExpr);
}

// Measures MoveOnlyFunction move construct against
// std::move_only_function's, small (SBO-fitting) capture, where
// available.
static void bench_move_only_move_small() {
    int a = kPad, b = kPad, c = kPad;

    auto cExpr = [&] {
        MoveOnlyFunction<int()> src = [a, b, c] { return a + b + c; };
        MoveOnlyFunction<int()> dst(std::move(src));
        doNotOptimize(dst);
    };

#if defined(__cpp_lib_move_only_function)
    auto sExpr = [&] {
        std::move_only_function<int()> src = [a, b, c] { return a + b + c; };
        std::move_only_function<int()> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("MoveOnlyFn move construct (small)", cExpr, sExpr);
#else
    BENCH_SOLO("MoveOnlyFn move construct (small)", cExpr);
#endif
}

// Measures MoveOnlyFunction move construct against
// std::move_only_function's, large (heap-forcing) capture, where
// available.
static void bench_move_only_move_large() {
    LargePayload payload{};

    auto cExpr = [&] {
        MoveOnlyFunction<int()> src = [payload] {
            (void)payload;
            return 1;
        };
        MoveOnlyFunction<int()> dst(std::move(src));
        doNotOptimize(dst);
    };

#if defined(__cpp_lib_move_only_function)
    auto sExpr = [&] {
        std::move_only_function<int()> src = [payload] {
            (void)payload;
            return 1;
        };
        std::move_only_function<int()> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("MoveOnlyFn move construct (large)", cExpr, sExpr);
#else
    BENCH_SOLO("MoveOnlyFn move construct (large)", cExpr);
#endif
}

// Measures FunctionRef move construct against std::function_ref's,
// where available. Not split by capture size — trivially-copyable
// pointer pair, flat regardless of the referenced callable.
static void bench_function_ref_move() {
    auto callable = [] { return 1; };

    auto cExpr = [&] {
        FunctionRef<int()> src(callable);
        FunctionRef<int()> dst(std::move(src));
        doNotOptimize(dst);
    };

#if defined(__cpp_lib_function_ref)
    auto sExpr = [&] {
        std::function_ref<int()> src(callable);
        std::function_ref<int()> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("FnRef move construct", cExpr, sExpr);
#else
    BENCH_SOLO("FnRef move construct", cExpr);
#endif
}

// Executes all move-semantics benchmark cases.
static void run_benchmarks() {
    bench_function_move_small();
    std::cout << "\n";

    bench_function_move_large();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::move_only_function");
    setHeader(suiteName);

    bench_move_only_move_small();
    std::cout << "\n";

    bench_move_only_move_large();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::function_ref");
    setHeader(suiteName);

    bench_function_ref_move();
}

REGISTER_BENCH_SUITE();
