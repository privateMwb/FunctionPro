// FunctionRef Benchmark Suite
// Compares FunctionRef against Function and std::function across
// construction, invocation, copy, rebinding, and pass-by-value costs.
//
// Covers:
// - functor construction
// - free function construction
// - functor invocation
// - free function invocation
// - lambda invocation
// - FunctionRef copy
// - FunctionRef rebind vs Function reassign
// - pass-by-value overhead

#include "bench_helper.h"

#include <functional>

using namespace FunctionPro;

// Free function used by construction and invocation benchmarks.
static int free_add(int a, int b) { return a + b; }

// Small callable used for functor-based benchmarks.
struct SmallCallable {
    int bias;
    int operator()(int a, int b) const { return a + b + bias; }
};

// Benchmarks construction from a functor.
static void bench_construct() {
    SmallCallable callable{1};

    auto ref_construct = [&] {
        FunctionRef<int(int,int)> f(callable);
        doNotOptimize(f);
    };
    BENCH("FunctionRef construct", LARGE, ref_construct);

    auto fp_construct = [&] {
        Function<int(int,int)> f(callable);
        doNotOptimize(f);
    };
    BENCH("Function construct", LARGE, fp_construct);

    auto std_construct = [&] {
        std::function<int(int,int)> f(callable);
        doNotOptimize(f);
    };
    BENCH("std::function construct", LARGE, std_construct);
}

// Benchmarks construction from a free function.
static void bench_construct_free_function() {
    auto ref_construct = [&] {
        FunctionRef<int(int,int)> f(free_add);
        doNotOptimize(f);
    };
    BENCH("FunctionRef construct free function", LARGE, ref_construct);

    auto fp_construct = [&] {
        Function<int(int,int)> f(free_add);
        doNotOptimize(f);
    };
    BENCH("Function construct free function", LARGE, fp_construct);

    auto std_construct = [&] {
        std::function<int(int,int)> f(free_add);
        doNotOptimize(f);
    };
    BENCH("std::function construct free function", LARGE, std_construct);
}

// Benchmarks functor invocation.
static void bench_invoke_functor() {
    SmallCallable               callable{1};
    FunctionRef<int(int,int)>   ref(callable);
    Function<int(int,int)>      fp(callable);
    std::function<int(int,int)> sf(callable);

    auto ref_invoke = [&] {
        int r = ref(1, 2);
        doNotOptimize(r);
    };
    BENCH("FunctionRef invoke functor", LARGE, ref_invoke);

    auto fp_invoke = [&] {
        int r = fp(1, 2);
        doNotOptimize(r);
    };
    BENCH("Function invoke functor", LARGE, fp_invoke);

    auto std_invoke = [&] {
        int r = sf(1, 2);
        doNotOptimize(r);
    };
    BENCH("std::function invoke functor", LARGE, std_invoke);
}

// Benchmarks free function invocation.
static void bench_invoke_free_function() {
    FunctionRef<int(int,int)>   ref(free_add);
    Function<int(int,int)>      fp(free_add);
    std::function<int(int,int)> sf(free_add);

    auto ref_invoke = [&] {
        int r = ref(1, 2);
        doNotOptimize(r);
    };
    BENCH("FunctionRef invoke free function", LARGE, ref_invoke);

    auto fp_invoke = [&] {
        int r = fp(1, 2);
        doNotOptimize(r);
    };
    BENCH("Function invoke free function", LARGE, fp_invoke);

    auto std_invoke = [&] {
        int r = sf(1, 2);
        doNotOptimize(r);
    };
    BENCH("std::function invoke free function", LARGE, std_invoke);
}

// Benchmarks lambda invocation.
static void bench_invoke_lambda() {
    auto lam = [bias = 1](int a, int b) { return a + b + bias; };
    FunctionRef<int(int,int)>   ref(lam);
    Function<int(int,int)>      fp(lam);
    std::function<int(int,int)> sf(lam);

    auto ref_invoke = [&] {
        int r = ref(1, 2);
        doNotOptimize(r);
    };
    BENCH("FunctionRef invoke lambda", LARGE, ref_invoke);

    auto fp_invoke = [&] {
        int r = fp(1, 2);
        doNotOptimize(r);
    };
    BENCH("Function invoke lambda", LARGE, fp_invoke);

    auto std_invoke = [&] {
        int r = sf(1, 2);
        doNotOptimize(r);
    };
    BENCH("std::function invoke lambda", LARGE, std_invoke);
}

// Benchmarks FunctionRef copy construction.
static void bench_copy() {
    SmallCallable             callable{1};
    FunctionRef<int(int,int)> src(callable);

    auto ref_copy = [&] {
        FunctionRef<int(int,int)> c(src);
        doNotOptimize(c);
    };
    BENCH("FunctionRef copy", LARGE, ref_copy);
}

// Benchmarks FunctionRef rebinding against Function reassignment.
static void bench_rebind() {
    SmallCallable callable_a{1};
    SmallCallable callable_b{2};

    auto ref_rebind = [&] {
        FunctionRef<int(int,int)> f(callable_a);
        f = FunctionRef<int(int,int)>(callable_b);
        doNotOptimize(f);
    };
    BENCH("FunctionRef rebind", LARGE, ref_rebind);

    auto fp_reassign = [&] {
        Function<int(int,int)> f(callable_a);
        f = callable_b;
        doNotOptimize(f);
    };
    BENCH("Function reassign", LARGE, fp_reassign);
}

// Benchmarks passing FunctionRef by value.
static void bench_pass_by_value() {
    SmallCallable             callable{1};
    FunctionRef<int(int,int)> ref(callable);

    int sum = 0;
    auto caller = [&](FunctionRef<int(int,int)> fn) {
        sum += fn(1, 2);
        doNotOptimize(sum);
    };

    auto ref_pass = [&] {
        caller(ref);
    };
    BENCH("FunctionRef pass by value", LARGE, ref_pass);
}

// Executes all FunctionRef benchmarks.
void run_function_ref_benchmarks() {
    setHeader("FunctionRef Benchmarks");

    setSubHeader("Construct Functor");
    bench_construct();
    std::cout << "\n";

    setSubHeader("Construct Free Function");
    bench_construct_free_function();
    std::cout << "\n";

    setSubHeader("Invoke Functor");
    bench_invoke_functor();
    std::cout << "\n";

    setSubHeader("Invoke Free Function");
    bench_invoke_free_function();
    std::cout << "\n";

    setSubHeader("Invoke Lambda");
    bench_invoke_lambda();
    std::cout << "\n";

    setSubHeader("Copy");
    bench_copy();
    std::cout << "\n";

    setSubHeader("Rebind vs Reassign");
    bench_rebind();
    std::cout << "\n";

    setSubHeader("Pass By Value");
    bench_pass_by_value();
    borderLine();
    std::cout << "\n";
}