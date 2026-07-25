// FunctionPro Null Compare Benchmark Suite
// Measures operator==(nullptr) performance for Function, MoveOnlyFunction,
// and FunctionRef against their std counterparts, on a bound (non-null)
// instance. Not split empty vs bound — same reasoning as
// access/bool_check.cpp: it's the same null-pointer check either way,
// not a genuinely different cost.
//
// FunctionRef is a permanent BENCH_SOLO here, not a toolchain-gated one:
// std::function_ref (P0792) is designed to be non-nullable — no default
// constructor, always bound to a valid callable at construction — so it
// has no operator==(nullptr_t) equivalent at all, regardless of
// toolchain version. Our FunctionRef explicitly supports an empty,
// default-constructed state, which is a real design difference worth
// noting rather than papering over with a feature-test gate.
//
// Covers:
// - Function::operator==(nullptr) vs std::function
// - MoveOnlyFunction::operator==(nullptr) vs std::move_only_function,
//   where available
// - FunctionRef::operator==(nullptr), solo — no std::function_ref
//   equivalent exists

#include <support/framework.h>

using namespace FunctionPro;

// Measures Function::operator==(nullptr) against std::function's.
static void bench_function_null_compare() {
    Function<int()> cSrc = [] { return 1; };
    std::function<int()> sSrc = [] { return 1; };

    auto c = [&] {
        bool v = (cSrc == nullptr);
        doNotOptimize(v);
    };

    auto s = [&] {
        bool v = (sSrc == nullptr);
        doNotOptimize(v);
    };

    BENCH("Fn::operator==(nullptr)", c, s);
}

// Measures MoveOnlyFunction::operator==(nullptr) against
// std::move_only_function's, where available.
static void bench_move_only_null_compare() {
    MoveOnlyFunction<int()> cSrc = [] { return 1; };

    auto c = [&] {
        bool v = (cSrc == nullptr);
        doNotOptimize(v);
    };

#if defined(__cpp_lib_move_only_function)
    std::move_only_function<int()> sSrc = [] { return 1; };
    auto s = [&] {
        bool v = (sSrc == nullptr);
        doNotOptimize(v);
    };
    BENCH("MoveOnlyFn::operator==(nullptr)", c, s);
#else
    BENCH_SOLO("MoveOnlyFn::operator==(nullptr)", c);
#endif
}

// Measures FunctionRef::operator==(nullptr), solo — std::function_ref
// has no equivalent (non-nullable by design, see file header).
static void bench_function_ref_null_compare() {
    auto callable = [] { return 1; };
    FunctionRef<int()> cSrc(callable);

    auto c = [&] {
        bool v = (cSrc == nullptr);
        doNotOptimize(v);
    };

    BENCH_SOLO("FnRef::operator==(nullptr)", c);
}

// Executes all null-compare benchmark cases.
static void run_benchmarks() {
    bench_function_null_compare();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::move_only_function");
    setHeader(suiteName);

    bench_move_only_null_compare();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::function_ref");
    setHeader(suiteName);

    bench_function_ref_null_compare();
}

REGISTER_BENCH_SUITE();
