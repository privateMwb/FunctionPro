// FunctionPro Construct/Destroy Benchmark Suite
// Measures default construction and immediate destruction of an empty
// instance, for all three types against their std counterparts.
//
// An empty instance has no callable to store or size, so there's no
// small/large split here — that axis only exists once a callable is
// bound. Populated construction (small vs large capture) lives in
// core/bind.cpp; this file isolates the floor cost of an empty object
// with no vtable/callable set up at all.
//
// Covers:
// - Function default construct + destroy vs std::function
// - MoveOnlyFunction default construct + destroy vs
//   std::move_only_function, where available
// - FunctionRef default construct + destroy vs std::function_ref,
//   where available

#include <support/framework.h>

using namespace FunctionPro;

// Measures default-constructing (and destroying) an empty Function
// against std::function's.
static void bench_function_construct_destroy() {
    auto c = [&] {
        Function<int()> f;
        doNotOptimize(f);
    };

    auto s = [&] {
        std::function<int()> f;
        doNotOptimize(f);
    };

    BENCH("Fn construct/destroy (empty)", c, s);
}

// Measures default-constructing (and destroying) an empty
// MoveOnlyFunction against std::move_only_function's, where available.
static void bench_move_only_construct_destroy() {
    auto c = [&] {
        MoveOnlyFunction<int()> f;
        doNotOptimize(f);
    };

#if defined(__cpp_lib_move_only_function)
    auto s = [&] {
        std::move_only_function<int()> f;
        doNotOptimize(f);
    };
    BENCH("MoveOnlyFn construct/destroy (empty)", c, s);
#else
    BENCH_SOLO("MoveOnlyFn construct/destroy (empty)", c);
#endif
}

// Measures default-constructing (and destroying) an empty FunctionRef
// against std::function_ref's, where available.
static void bench_function_ref_construct_destroy() {
    auto c = [&] {
        FunctionRef<int()> f;
        doNotOptimize(f);
    };

#if defined(__cpp_lib_function_ref)
    auto s = [&] {
        std::function_ref<int()> f;
        doNotOptimize(f);
    };
    BENCH("FnRef construct/destroy (empty)", c, s);
#else
    BENCH_SOLO("FnRef construct/destroy (empty)", c);
#endif
}

// Executes all construct/destroy benchmark cases.
static void run_benchmarks() {
    bench_function_construct_destroy();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::move_only_function");
    setHeader(suiteName);

    bench_move_only_construct_destroy();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::function_ref");
    setHeader(suiteName);

    bench_function_ref_construct_destroy();
}

REGISTER_BENCH_SUITE();
