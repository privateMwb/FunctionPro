// FunctionRef Benchmark Suite
// Evaluates FunctionPro::FunctionRef against std::function and raw function
// pointers across construction, invocation, copy, and rebinding scenarios.

#include "benchmark_helper.h"
#include "../include/function/FunctionRef.h"

#include <functional>

using namespace FunctionPro;

// Construct FunctionRef
// Measures the cost of binding FunctionRef to callable targets.
static void construct() {
    SmallCallable c{ 1 };
    BENCH("FunctionRef construct lambda", ITERATIONS, FunctionRef<int(int, int)>(c));
    BENCH("FunctionRef construct free function", ITERATIONS, FunctionRef<int(int, int)>(free_add));
}

// Invoke Free Function
// Compares invocation overhead against std::function and raw function pointers.
static void invoke_free() {
    FunctionRef<int(int, int)>    ref(free_add);
    std::function<int(int, int)>  sf(free_add);
    int (*fp)(int, int) = free_add;
    BENCH("FunctionRef invoke free", ITERATIONS, ref(1, 2));
    BENCH("std::function invoke free", ITERATIONS, sf(1, 2));
    BENCH("raw pointer invoke free", ITERATIONS, fp(1, 2));
}

// Invoke SBO Callable
// Measures dispatch cost when referencing a small callable object.
static void invoke_sbo() {
    SmallCallable c{ 1 };
    FunctionRef<int(int, int)>    ref(c);
    std::function<int(int, int)>  sf(c);
    BENCH("FunctionRef invoke SBO", ITERATIONS, ref(1, 2));
    BENCH("std::function invoke SBO", ITERATIONS, sf(1, 2));
}

// Invoke Void Callable
// Measures invocation performance for callables returning no value.
static void invoke_void() {
    int x = 0;
    FunctionRef<void(int&)>      ref(free_side);
    std::function<void(int&)>    sf(free_side);
    BENCH("FunctionRef invoke void", ITERATIONS, ref(x));
    BENCH("std::function invoke void", ITERATIONS, sf(x));
}

// Copy FunctionRef
// Measures the cost of copying a lightweight function reference.
static void copy() {
    SmallCallable c{ 1 };
    FunctionRef<int(int, int)> src(c);
    BENCH("FunctionRef copy", ITERATIONS, FunctionRef<int(int, int)>(src));
}

// Rebind FunctionRef
// Measures the cost of redirecting a FunctionRef to a new target.
static void rebind() {
    SmallCallable c1{ 1 };
    SmallCallable c2{ 2 };
    FunctionRef<int(int, int)> ref(c1);
    BENCH("FunctionRef rebind", ITERATIONS, (ref = FunctionRef<int(int, int)>(c2)));
}

// Run All Benchmarks
// Executes the complete FunctionRef benchmark suite.
void run_function_ref_benchmarks() {
    construct();
    std::cout << "\n";

    invoke_free();
    std::cout << "\n";

    invoke_sbo();
    std::cout << "\n";

    invoke_void();
    std::cout << "\n";

    copy();
    std::cout << "\n";

    rebind();
    std::cout << "\n";
}