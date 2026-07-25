// FunctionPro Bool Check Benchmark Suite
// Measures operator bool() performance for Function, MoveOnlyFunction,
// and FunctionRef against their std counterparts, on a bound (non-null)
// instance.
//
// The file's default comparison is FunctionPro vs std::function, so the
// Function case below needs no label override. MoveOnlyFunction and
// FunctionRef compare against different baselines, so each overrides
// the project labels and header before its own BENCH/BENCH_SOLO call.
//
// Covers:
// - Function::operator bool() vs std::function::operator bool()
// - MoveOnlyFunction::operator bool() vs
//   std::move_only_function::operator bool() (or BENCH_SOLO on
//   toolchains without std::move_only_function yet)
// - FunctionRef::operator bool() vs std::function_ref::operator bool()
//   (or BENCH_SOLO on toolchains without std::function_ref yet)

#include <support/framework.h>

using namespace FunctionPro;

// Measures Function::operator bool() against std::function's.
static void bench_function_bool() {
    Function<int()> cSrc = [] { return 1; };
    std::function<int()> sSrc = [] { return 1; };

    auto c = [&] {
        bool v = static_cast<bool>(cSrc);
        doNotOptimize(v);
    };

    auto s = [&] {
        bool v = static_cast<bool>(sSrc);
        doNotOptimize(v);
    };

    BENCH("Fn::operator bool()", c, s);
}

// Measures MoveOnlyFunction::operator bool() against
// std::move_only_function's, where available.
static void bench_move_only_bool() {
    MoveOnlyFunction<int()> cSrc = [] { return 1; };

    auto c = [&] {
        bool v = static_cast<bool>(cSrc);
        doNotOptimize(v);
    };

#if defined(__cpp_lib_move_only_function)
    std::move_only_function<int()> sSrc = [] { return 1; };
    auto s = [&] {
        bool v = static_cast<bool>(sSrc);
        doNotOptimize(v);
    };
    BENCH("MoveOnlyFn::operator bool()", c, s);
#else
    BENCH_SOLO("MoveOnlyFn::operator bool()", c);
#endif
}

// Measures FunctionRef::operator bool() against std::function_ref's,
// where available.
static void bench_function_ref_bool() {
    auto callable = [] { return 1; };
    FunctionRef<int()> cSrc(callable);

    auto c = [&] {
        bool v = static_cast<bool>(cSrc);
        doNotOptimize(v);
    };

#if defined(__cpp_lib_function_ref)
    std::function_ref<int()> sSrc(callable);
    auto s = [&] {
        bool v = static_cast<bool>(sSrc);
        doNotOptimize(v);
    };
    BENCH("FnRef::operator bool()", c, s);
#else
    BENCH_SOLO("FnRef::operator bool()", c);
#endif
}

// Executes all bool-check benchmark cases.
static void run_benchmarks() {
    bench_function_bool();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::move_only_function");
    setHeader(suiteName);

    bench_move_only_bool();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::function_ref");
    setHeader(suiteName);

    bench_function_ref_bool();
}

REGISTER_BENCH_SUITE();
