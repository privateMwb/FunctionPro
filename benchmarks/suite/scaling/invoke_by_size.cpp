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

#include <support/framework.h>

#include <array>

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
// size, against std::function's, hit path.
template <std::size_t N> static void bench_invoke_by_size(const char* label) {
    SizedCallable<N> callable{};
    Function<int()> cSrc(callable);
    std::function<int()> sSrc(callable);

    auto cExpr = [&] {
        int v = cSrc();
        doNotOptimize(v);
    };

    auto sExpr = [&] {
        int v = sSrc();
        doNotOptimize(v);
    };

    BENCH(label, cExpr, sExpr);
}

// Executes all invoke-by-size benchmark cases.
static void run_benchmarks() {
    bench_invoke_by_size<0>("Fn invoke (0B)");
    std::cout << "\n";

    bench_invoke_by_size<16>("Fn invoke (16B)");
    std::cout << "\n";

    bench_invoke_by_size<40>("Fn invoke (40B, SBO boundary)");
    std::cout << "\n";

    bench_invoke_by_size<64>("Fn invoke (64B, heap)");
    std::cout << "\n";

    bench_invoke_by_size<256>("Fn invoke (256B, heap)");
}

REGISTER_BENCH_SUITE();
