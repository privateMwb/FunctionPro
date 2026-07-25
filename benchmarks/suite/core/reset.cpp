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
//   where available

#include <support/framework.h>

#include <array>

using namespace FunctionPro;

namespace {
// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit,
// so reset() has an actual heap deallocation to perform each call.
struct LargePayload {
    std::array<std::byte, 64> padding{};
};
} // namespace

// Measures Function::reset() against std::function's `f = nullptr`.
static void bench_function_reset() {
    LargePayload payload{};

    auto cExpr = [&] {
        Function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        f.reset();
        doNotOptimize(f);
    };

    auto sExpr = [&] {
        std::function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        f = nullptr;
        doNotOptimize(f);
    };

    BENCH("Fn::reset()", cExpr, sExpr);
}

// Measures MoveOnlyFunction::reset() against std::move_only_function's
// `f = nullptr`, where available.
static void bench_move_only_reset() {
    LargePayload payload{};

    auto cExpr = [&] {
        MoveOnlyFunction<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        f.reset();
        doNotOptimize(f);
    };

#if defined(__cpp_lib_move_only_function)
    auto sExpr = [&] {
        std::move_only_function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        f = nullptr;
        doNotOptimize(f);
    };
    BENCH("MoveOnlyFn::reset()", cExpr, sExpr);
#else
    BENCH_SOLO("MoveOnlyFn::reset()", cExpr);
#endif
}

// Executes all reset benchmark cases.
static void run_benchmarks() {
    bench_function_reset();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::move_only_function");
    setHeader(suiteName);

    bench_move_only_reset();
}

REGISTER_BENCH_SUITE();
