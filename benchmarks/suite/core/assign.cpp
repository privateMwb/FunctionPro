// FunctionPro Assign Benchmark Suite
// Measures copy-assign and move-assign performance across Function,
// MoveOnlyFunction, and FunctionRef against their std counterparts.
//
// Function gets both a copy-assign and a move-assign case — these are
// genuinely different code paths (copy-assign goes through a
// copy-construct-then-swap for the strong exception guarantee;
// move-assign goes through the vtable's move operation directly).
// MoveOnlyFunction gets move-assign only — copy-assign is deleted, it
// doesn't exist. FunctionRef gets a single "assign" case, not split
// into copy/move variants: it's a trivially-copyable pointer pair, so
// its copy-assign and move-assign compile to the identical instruction
// sequence — labeling both would be the same benchmark twice.
//
// Move-assign cases ping-pong a populated instance between two slots
// (a -> b, then b -> a, ...) so no per-call rebuild cost leaks into the
// measurement — only the move itself is timed.
//
// Covers:
// - Function copy-assign vs std::function
// - Function move-assign vs std::function
// - MoveOnlyFunction move-assign vs std::move_only_function, where available
// - FunctionRef assign vs std::function_ref, where available

#include <support/framework.h>

using namespace FunctionPro;

// Measures Function copy-assign against std::function's.
static void bench_function_copy_assign() {
    Function<int()> cSrc = [] { return 1; };
    std::function<int()> sSrc = [] { return 1; };
    Function<int()> cDst;
    std::function<int()> sDst;

    auto c = [&] { cDst = cSrc; };
    auto s = [&] { sDst = sSrc; };

    BENCH("Fn copy-assign", c, s);
}

// Measures Function move-assign against std::function's, ping-ponging
// between two slots.
static void bench_function_move_assign() {
    Function<int()> cA = [] { return 1; };
    Function<int()> cB;
    std::function<int()> sA = [] { return 1; };
    std::function<int()> sB;
    bool cFlip = false;
    bool sFlip = false;

    auto c = [&] {
        if (!cFlip)
            cB = std::move(cA);
        else
            cA = std::move(cB);
        cFlip = !cFlip;
    };
    auto s = [&] {
        if (!sFlip)
            sB = std::move(sA);
        else
            sA = std::move(sB);
        sFlip = !sFlip;
    };

    BENCH("Fn move-assign", c, s);
}

// Measures MoveOnlyFunction move-assign against
// std::move_only_function's, ping-ponging between two slots, where
// available.
static void bench_move_only_move_assign() {
    MoveOnlyFunction<int()> cA = [] { return 1; };
    MoveOnlyFunction<int()> cB;
    bool cFlip = false;

    auto c = [&] {
        if (!cFlip)
            cB = std::move(cA);
        else
            cA = std::move(cB);
        cFlip = !cFlip;
    };

#if defined(__cpp_lib_move_only_function)
    std::move_only_function<int()> sA = [] { return 1; };
    std::move_only_function<int()> sB;
    bool sFlip = false;
    auto s = [&] {
        if (!sFlip)
            sB = std::move(sA);
        else
            sA = std::move(sB);
        sFlip = !sFlip;
    };
    BENCH("MoveOnlyFn move-assign", c, s);
#else
    BENCH_SOLO("MoveOnlyFn move-assign", c);
#endif
}

// Measures FunctionRef's trivial assign against std::function_ref's,
// where available. Not split into copy/move — both compile to the same
// instruction sequence for a trivially-copyable pointer pair.
static void bench_function_ref_assign() {
    auto callable = [] { return 1; };
    FunctionRef<int()> cSrc(callable);
    FunctionRef<int()> cDst;

    auto c = [&] { cDst = cSrc; };

#if defined(__cpp_lib_function_ref)
    std::function_ref<int()> sSrc(callable);
    std::function_ref<int()> sDst = sSrc;
    auto s = [&] { sDst = sSrc; };
    BENCH("FnRef assign", c, s);
#else
    BENCH_SOLO("FnRef assign", c);
#endif
}

// Executes all assign benchmark cases.
static void run_benchmarks() {
    bench_function_copy_assign();
    std::cout << "\n";

    bench_function_move_assign();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::move_only_function");
    setHeader(suiteName);

    bench_move_only_move_assign();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::function_ref");
    setHeader(suiteName);

    bench_function_ref_assign();
}

REGISTER_BENCH_SUITE();
