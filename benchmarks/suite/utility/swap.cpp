// FunctionPro Swap Benchmark Suite
// Measures swap() performance for Function and MoveOnlyFunction against
// their std counterparts. FunctionRef has no swap() member at all — not
// even a solo case, the operation simply doesn't exist on it — so it's
// excluded entirely.
//
// swap() deliberately goes through the vtable's move operation on both
// sides rather than exchanging raw storage bytes, so this is a real,
// measurable cost distinct from a hypothetical byte-swap — not the same
// benchmark as move-assign (core/assign.cpp) under a different name,
// since swap does two moves in sequence rather than one.
//
// Covers:
// - Function::swap() vs std::function::swap()
// - MoveOnlyFunction::swap() vs std::move_only_function::swap(), where
//   available

#include <support/framework.h>

using namespace FunctionPro;

// Measures Function::swap() against std::function::swap().
static void bench_function_swap() {
    Function<int()> cA = [] { return 1; };
    Function<int()> cB = [] { return 2; };
    std::function<int()> sA = [] { return 1; };
    std::function<int()> sB = [] { return 2; };

    auto c = [&] { cA.swap(cB); };
    auto s = [&] { sA.swap(sB); };

    BENCH("Fn::swap()", c, s);
}

// Measures MoveOnlyFunction::swap() against
// std::move_only_function::swap(), where available.
static void bench_move_only_swap() {
    MoveOnlyFunction<int()> cA = [] { return 1; };
    MoveOnlyFunction<int()> cB = [] { return 2; };

    auto c = [&] { cA.swap(cB); };

#if defined(__cpp_lib_move_only_function)
    std::move_only_function<int()> sA = [] { return 1; };
    std::move_only_function<int()> sB = [] { return 2; };
    auto s = [&] { sA.swap(sB); };
    BENCH("MoveOnlyFn::swap()", c, s);
#else
    BENCH_SOLO("MoveOnlyFn::swap()", c);
#endif
}

// Executes all swap benchmark cases.
static void run_benchmarks() {
    bench_function_swap();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::move_only_function");
    setHeader(suiteName);

    bench_move_only_swap();
}

REGISTER_BENCH_SUITE();
