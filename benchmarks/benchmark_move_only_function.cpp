// MoveOnlyFunction Benchmark Suite
// Evaluates MoveOnlyFunction against std::function across construction,
// invocation, move operations, and move-only callable support.

#include "benchmark_helper.h"
#include "../include/function/MoveOnlyFunction.h"

#include <functional>
#include <memory>

using namespace FunctionPro;

// Construct SBO Callable
// Measures construction cost for small-buffer optimized callables.
static void construct_sbo() {
    BENCH("MoveOnlyFunction construct SBO", ITERATIONS, MoveOnlyFunction<int(int, int)>(SmallCallable{ 1 }));
    BENCH("std::function construct SBO", ITERATIONS, std::function<int(int, int)>(SmallCallable{ 1 }));
}

// Construct Heap Callable
// Measures construction cost for heap-allocated callable objects.
static void construct_heap() {
    BENCH("MoveOnlyFunction construct heap", ITERATIONS, MoveOnlyFunction<int(int, int)>(LargeCallable{}));
    BENCH("std::function construct heap", ITERATIONS, std::function<int(int, int)>(LargeCallable{}));
}

// Invoke Free Function
// Compares invocation overhead for wrapped free functions.
static void invoke_free() {
    MoveOnlyFunction<int(int, int)> f(free_add);
    std::function<int(int, int)> sf(free_add);
    BENCH("MoveOnlyFunction invoke free", ITERATIONS, f(1, 2));
    BENCH("std::function invoke free", ITERATIONS, sf(1, 2));
}

// Invoke SBO Callable
// Measures dispatch performance for small callable objects.
static void invoke_sbo() {
    MoveOnlyFunction<int(int, int)> f(SmallCallable{ 1 });
    std::function<int(int, int)> sf(SmallCallable{ 1 });
    BENCH("MoveOnlyFunction invoke SBO", ITERATIONS, f(1, 2));
    BENCH("std::function invoke SBO", ITERATIONS, sf(1, 2));
}

// Invoke Heap Callable
// Measures dispatch performance for heap-allocated callables.
static void invoke_heap() {
    MoveOnlyFunction<int(int, int)> f(LargeCallable{});
    std::function<int(int, int)> sf(LargeCallable{});
    BENCH("MoveOnlyFunction invoke heap", ITERATIONS, f(1, 2));
    BENCH("std::function invoke heap", ITERATIONS, sf(1, 2));
}

// Invoke Move-Only Callable
// Validates support and performance for move-only lambda captures.
static void invoke_moveonly() {
    MoveOnlyFunction<int()> f(
        [p = std::make_unique<int>(42)]() {
            return *p;
        }
    );

    BENCH("MoveOnlyFunction invoke move-only", ITERATIONS, f());
}

// Move SBO Callable
// Measures move-construction cost for SBO-backed callables.
static void move_sbo() {
    BENCH(
        "MoveOnlyFunction move SBO",
        ITERATIONS,
        ([]() {
            MoveOnlyFunction<int(int, int)> src(SmallCallable{ 1 });
            MoveOnlyFunction<int(int, int)> dst(std::move(src));
            return dst;
            }())
    );

    BENCH(
        "std::function move SBO",
        ITERATIONS,
        ([]() {
            std::function<int(int, int)> src(SmallCallable{ 1 });
            std::function<int(int, int)> dst(std::move(src));
            return dst;
            }())
    );
}

// Move Heap Callable
// Measures move-construction cost for heap-allocated callables.
static void move_heap() {
    BENCH(
        "MoveOnlyFunction move heap",
        ITERATIONS,
        ([]() {
            MoveOnlyFunction<int(int, int)> src(LargeCallable{});
            MoveOnlyFunction<int(int, int)> dst(std::move(src));
            return dst;
            }())
    );

    BENCH(
        "std::function move heap",
        ITERATIONS,
        ([]() {
            std::function<int(int, int)> src(LargeCallable{});
            std::function<int(int, int)> dst(std::move(src));
            return dst;
            }())
    );
}

// Run All Benchmarks
// Executes the complete MoveOnlyFunction benchmark suite.
void run_move_only_function_benchmarks() {
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

    invoke_moveonly();
    std::cout << "\n";

    move_sbo();
    std::cout << "\n";

    move_heap();
    std::cout << "\n";
}