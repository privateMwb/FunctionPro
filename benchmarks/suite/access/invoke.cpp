// FunctionPro Invoke Benchmark Suite
// Measures operator() performance for Function, MoveOnlyFunction, and
// FunctionRef against their std counterparts.
//
// Each hit-path case binds a small, SBO-fitting capturing lambda once,
// outside the timed lambda — only repeated invocation is measured.
// Invoke cost as capture size grows past the SBO threshold is covered
// separately in scaling/invoke_by_size.cpp.
//
// The file's default comparison is FunctionPro vs std::function, so
// Function cases below need no label override. MoveOnlyFunction and
// FunctionRef compare against different baselines, so each overrides
// the project labels and header before its own BENCH/BENCH_SOLO call.
//
// Covers:
// - operator() hit path, for all three types against their std
//   counterparts (or BENCH_SOLO where the std type isn't available)
// - operator() on an empty instance — all three throw
//   std::bad_function_call, so this times the throw path itself
// - FunctionRef::operator() bound to a raw function pointer vs bound to
//   a capturing lambda — FunctionRef special-cases function pointers
//   internally to skip an extra indirection, so this isolates that path

#include <support/framework.h>

using namespace FunctionPro;

namespace {
constexpr int kAddend = 7;

int addSeven(int x) {
    return x + kAddend;
}
} // namespace

// Measures Function::operator() against std::function::operator(),
// hit path.
static void bench_function_invoke() {
    int captured = kAddend;
    Function<int(int)> cSrc = [captured](int x) { return x + captured; };
    std::function<int(int)> sSrc = [captured](int x) { return x + captured; };

    auto c = [&] {
        int v = cSrc(1);
        doNotOptimize(v);
    };

    auto s = [&] {
        int v = sSrc(1);
        doNotOptimize(v);
    };

    BENCH("Fn::operator()", c, s);
}

// Measures Function::operator() on an empty instance against
// std::function's — both throw std::bad_function_call.
static void bench_function_invoke_empty() {
    Function<int(int)> cSrc;
    std::function<int(int)> sSrc;

    auto c = [&] {
        try {
            int v = cSrc(1);
            doNotOptimize(v);
        } catch (const std::bad_function_call&) {
        }
    };

    auto s = [&] {
        try {
            int v = sSrc(1);
            doNotOptimize(v);
        } catch (const std::bad_function_call&) {
        }
    };

    BENCH("Fn::operator() (empty)", c, s);
}

// Measures MoveOnlyFunction::operator() against
// std::move_only_function::operator(), hit path, where available.
static void bench_move_only_invoke() {
    int captured = kAddend;
    MoveOnlyFunction<int(int)> cSrc = [captured](int x) { return x + captured; };

    auto c = [&] {
        int v = cSrc(1);
        doNotOptimize(v);
    };

#if defined(__cpp_lib_move_only_function)
    std::move_only_function<int(int)> sSrc = [captured](int x) { return x + captured; };
    auto s = [&] {
        int v = sSrc(1);
        doNotOptimize(v);
    };
    BENCH("MoveOnlyFn::operator()", c, s);
#else
    BENCH_SOLO("MoveOnlyFn::operator()", c);
#endif
}

// Measures MoveOnlyFunction::operator() on an empty instance.
// No std::move_only_function comparison here: unlike std::function,
// invoking an empty std::move_only_function is undefined behavior
// (not guaranteed to throw), so there's no safe reference-side case.
static void bench_move_only_invoke_empty() {
    MoveOnlyFunction<int(int)> cSrc;

    auto c = [&] {
        try {
            int v = cSrc(1);
            doNotOptimize(v);
        } catch (const std::bad_function_call&) {
        }
    };

    BENCH_SOLO("MoveOnlyFn::operator() (empty)", c);
}

// Measures FunctionRef::operator() against std::function_ref::operator(),
// hit path, bound to a capturing lambda, where available.
static void bench_function_ref_invoke() {
    int captured = kAddend;
    auto callable = [captured](int x) { return x + captured; };
    FunctionRef<int(int)> cSrc(callable);

    auto c = [&] {
        int v = cSrc(1);
        doNotOptimize(v);
    };

#if defined(__cpp_lib_function_ref)
    std::function_ref<int(int)> sSrc(callable);
    auto s = [&] {
        int v = sSrc(1);
        doNotOptimize(v);
    };
    BENCH("FnRef::operator()", c, s);
#else
    BENCH_SOLO("FnRef::operator()", c);
#endif
}

// Measures FunctionRef::operator() on an empty instance against
// std::function_ref's, where available.
static void bench_function_ref_invoke_empty() {
    FunctionRef<int(int)> cSrc;

    auto c = [&] {
        try {
            int v = cSrc(1);
            doNotOptimize(v);
        } catch (const std::bad_function_call&) {
        }
    };

#if defined(__cpp_lib_function_ref)
    std::function_ref<int(int)> sSrc;
    auto s = [&] {
        try {
            int v = sSrc(1);
            doNotOptimize(v);
        } catch (const std::bad_function_call&) {
        }
    };
    BENCH("FnRef::operator() (empty)", c, s);
#else
    BENCH_SOLO("FnRef::operator() (empty)", c);
#endif
}

// Measures FunctionRef::operator() bound to a raw function pointer
// against std::function_ref's, isolating the function-pointer
// fast path FunctionRef special-cases internally to skip storing (and
// indirecting through) an object pointer.
static void bench_function_ref_invoke_fnptr() {
    FunctionRef<int(int)> cSrc(addSeven);

    auto c = [&] {
        int v = cSrc(1);
        doNotOptimize(v);
    };

#if defined(__cpp_lib_function_ref)
    std::function_ref<int(int)> sSrc(addSeven);
    auto s = [&] {
        int v = sSrc(1);
        doNotOptimize(v);
    };
    BENCH("FnRef::operator() (function pointer)", c, s);
#else
    BENCH_SOLO("FnRef::operator() (function pointer)", c);
#endif
}

// Executes all invoke benchmark cases.
static void run_benchmarks() {
    bench_function_invoke();
    std::cout << "\n";

    bench_function_invoke_empty();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::move_only_function");
    setHeader(suiteName);

    bench_move_only_invoke();
    std::cout << "\n";

    bench_move_only_invoke_empty();
    std::cout << "\n";

    setProjectLabels("FunctionPro", "std::function_ref");
    setHeader(suiteName);

    bench_function_ref_invoke();
    std::cout << "\n";

    bench_function_ref_invoke_empty();
    std::cout << "\n";

    bench_function_ref_invoke_fnptr();
}

REGISTER_BENCH_SUITE();
