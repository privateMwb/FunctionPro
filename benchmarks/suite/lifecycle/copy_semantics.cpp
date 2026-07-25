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

// Measures Function copy construct against std::function's, small
// (SBO-fitting) capture.
static void bench_function_copy_small() {
    int a = kPad, b = kPad, c = kPad;
    Function<int()> cSrc = [a, b, c] { return a + b + c; };
    std::function<int()> sSrc = [a, b, c] { return a + b + c; };

    auto cExpr = [&] {
        Function<int()> f(cSrc);
        doNotOptimize(f);
    };

    auto sExpr = [&] {
        std::function<int()> f(sSrc);
        doNotOptimize(f);
    };

    BENCH("Fn copy construct (small)", cExpr, sExpr);
}

// Measures Function copy construct against std::function's, large
// (heap-forcing) capture.
static void bench_function_copy_large() {
    LargePayload payload{};
    Function<int()> cSrc = [payload] {
        (void)payload;
        return 1;
    };
    std::function<int()> sSrc = [payload] {
        (void)payload;
        return 1;
    };

    auto cExpr = [&] {
        Function<int()> f(cSrc);
        doNotOptimize(f);
    };

    auto sExpr = [&] {
        std::function<int()> f(sSrc);
        doNotOptimize(f);
    };

    BENCH("Fn copy construct (large)", cExpr, sExpr);
}

// Measures FunctionRef copy construct against std::function_ref's,
// where available. Not split by capture size — the copy is a flat,
// trivial pointer-pair copy regardless of the referenced callable.
static void bench_function_ref_copy() {
    auto callable = [] { return 1; };
    FunctionRef<int()> cSrc(callable);

    auto cExpr = [&] {
        FunctionRef<int()> f(cSrc);
        doNotOptimize(f);
    };

#if defined(__cpp_lib_function_ref)
    std::function_ref<int()> sSrc(callable);
    auto sExpr = [&] {
        std::function_ref<int()> f(sSrc);
        doNotOptimize(f);
    };
    BENCH("FnRef copy construct", cExpr, sExpr);
#else
    BENCH_SOLO("FnRef copy construct", cExpr);
#endif
}

// Executes all copy-semantics benchmark cases.
static void run_benchmarks() {
    bench_function_copy_small();
    std::cout << "\n";

    bench_function_copy_large();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::function_ref");
    setHeader(suiteName);

    bench_function_ref_copy();
}

REGISTER_BENCH_SUITE();
