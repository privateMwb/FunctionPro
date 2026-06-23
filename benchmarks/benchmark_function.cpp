// Function Benchmark Suite
// Compares FunctionPro::Function against std::function across construction,
// invocation, copy, and move operations for SBO and heap-allocated callables.

#include "benchmark_helper.h"
#include "../include/function/Function.h"

#include <functional>

using namespace FunctionPro;

// Construct SBO Callable
// Measures construction cost for small callables stored using Small Buffer Optimization.
static void construct_sbo() {
    BENCH("Function construct SBO", ITERATIONS, Function<int(int, int)>(SmallCallable{ 1 }));
    BENCH("std::function construct SBO", ITERATIONS, std::function<int(int, int)>(SmallCallable{ 1 }));
}

// Construct Heap Callable
// Measures construction cost for large callables requiring dynamic allocation.
static void construct_heap() {
    BENCH("Function construct heap", ITERATIONS, Function<int(int, int)>(LargeCallable{}));
    BENCH("std::function construct heap", ITERATIONS, std::function<int(int, int)>(LargeCallable{}));
}

// Invoke Free Function
// Measures dispatch overhead when invoking a plain function pointer.
static void invoke_free() {
    Function<int(int, int)>       f(free_add);
    std::function<int(int, int)>  sf(free_add);
    BENCH("Function invoke free", ITERATIONS, f(1, 2));
    BENCH("std::function invoke free", ITERATIONS, sf(1, 2));
}

// Invoke SBO Callable
// Measures invocation cost for callables stored directly inside the function object.
static void invoke_sbo() {
    Function<int(int, int)>       f(SmallCallable{ 1 });
    std::function<int(int, int)>  sf(SmallCallable{ 1 });
    BENCH("Function invoke SBO", ITERATIONS, f(1, 2));
    BENCH("std::function invoke SBO", ITERATIONS, sf(1, 2));
}

// Invoke Heap Callable
// Measures invocation cost for dynamically allocated callable targets.
static void invoke_heap() {
    Function<int(int, int)>       f(LargeCallable{});
    std::function<int(int, int)>  sf(LargeCallable{});
    BENCH("Function invoke heap", ITERATIONS, f(1, 2));
    BENCH("std::function invoke heap", ITERATIONS, sf(1, 2));
}

// Copy Function Wrapper
// Measures overhead of duplicating an existing function wrapper instance.
static void copy() {
    Function<int(int, int)>       src(SmallCallable{ 1 });
    std::function<int(int, int)>  ssrc(SmallCallable{ 1 });
    BENCH("Function copy", ITERATIONS, Function<int(int, int)>(src));
    BENCH("std::function copy", ITERATIONS, std::function<int(int, int)>(ssrc));
}

// Move Function Wrapper
// Measures transfer cost when moving ownership between function wrappers.
static void move() {
    BENCH("Function move",
        ITERATIONS,
        ([]() {
            Function<int(int, int)> src(SmallCallable{ 1 });
            Function<int(int, int)> dst(std::move(src));
            return dst;
            }())
    );

    BENCH("std::function move",
        ITERATIONS,
        ([]() {
            std::function<int(int, int)> src(SmallCallable{ 1 });
            std::function<int(int, int)> dst(std::move(src));
            }())
    );
}

// Run All Benchmarks
// Executes the complete benchmark suite and prints grouped results.
void run_function_benchmarks() {
    construct_sbo();
    std::cout << "\n";

    construct_heap();
    std::cout << "\n";

    invoke_free();
    std::cout << "\n";

    invoke_sbo();
    std::cout << "\n";

    invoke_heap();
    std::cout << "\n";

    copy();
    std::cout << "\n";

    move();
}