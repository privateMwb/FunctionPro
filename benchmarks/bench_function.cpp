#include "bench_helper.h"

#include <functional>

using namespace FunctionPro;

static int free_add(int a, int b) { return a + b; }

// Small callable that fits within the small-buffer optimization.
struct SmallCallable {
    int bias;
    int operator()(int a, int b) const { return a + b + bias; }
};

// Large callable that exceeds the small-buffer optimization capacity.
struct LargeCallable {
    std::byte pad[64] = {};
    int operator()(int a, int b) const { return a + b; }
};

// Benchmarks construction of SBO-sized callables.
static void bench_construct_sbo() {
    auto fp_sbo = [&] {
        Function<int(int,int)> f(SmallCallable{1});
        doNotOptimize(f);
    };
    BENCH("Function construct SBO", MEDIUM, fp_sbo);

    auto std_sbo = [&] {
        std::function<int(int,int)> f(SmallCallable{1});
        doNotOptimize(f);
    };
    BENCH("std::function construct SBO", MEDIUM, std_sbo);
}

// Benchmarks construction of heap-allocated callables.
static void bench_construct_heap() {
    auto fp_heap = [&] {
        Function<int(int,int)> f(LargeCallable{});
        doNotOptimize(f);
    };
    BENCH("Function construct heap", MEDIUM, fp_heap);

    auto std_heap = [&] {
        std::function<int(int,int)> f(LargeCallable{});
        doNotOptimize(f);
    };
    BENCH("std::function construct heap", MEDIUM, std_heap);
}

// Benchmarks invocation of free functions.
static void bench_invoke_free() {
    Function<int(int,int)>      f(free_add);
    std::function<int(int,int)> sf(free_add);

    auto fp_invoke = [&] {
        int r = f(1, 2);
        doNotOptimize(r);
    };
    BENCH("Function invoke free", LARGE, fp_invoke);

    auto std_invoke = [&] {
        int r = sf(1, 2);
        doNotOptimize(r);
    };
    BENCH("std::function invoke free", LARGE, std_invoke);
}

// Benchmarks invocation of SBO-stored callables.
static void bench_invoke_sbo() {
    Function<int(int,int)>      f(SmallCallable{1});
    std::function<int(int,int)> sf(SmallCallable{1});

    auto fp_invoke = [&] {
        int r = f(1, 2);
        doNotOptimize(r);
    };
    BENCH("Function invoke SBO", LARGE, fp_invoke);

    auto std_invoke = [&] {
        int r = sf(1, 2);
        doNotOptimize(r);
    };
    BENCH("std::function invoke SBO", LARGE, std_invoke);
}

// Benchmarks invocation of heap-stored callables.
static void bench_invoke_heap() {
    Function<int(int,int)>      f(LargeCallable{});
    std::function<int(int,int)> sf(LargeCallable{});

    auto fp_invoke = [&] {
        int r = f(1, 2);
        doNotOptimize(r);
    };
    BENCH("Function invoke heap", LARGE, fp_invoke);

    auto std_invoke = [&] {
        int r = sf(1, 2);
        doNotOptimize(r);
    };
    BENCH("std::function invoke heap", LARGE, std_invoke);
}

// Benchmarks copying SBO-stored callables.
static void bench_copy_sbo() {
    Function<int(int,int)>      src(SmallCallable{1});
    std::function<int(int,int)> ssrc(SmallCallable{1});

    auto fp_copy = [&] {
        Function<int(int,int)> c(src);
        doNotOptimize(c);
    };
    BENCH("Function copy SBO", MEDIUM, fp_copy);

    auto std_copy = [&] {
        std::function<int(int,int)> c(ssrc);
        doNotOptimize(c);
    };
    BENCH("std::function copy SBO", MEDIUM, std_copy);
}

// Benchmarks copying heap-stored callables.
static void bench_copy_heap() {
    Function<int(int,int)>      src(LargeCallable{});
    std::function<int(int,int)> ssrc(LargeCallable{});

    auto fp_copy = [&] {
        Function<int(int,int)> c(src);
        doNotOptimize(c);
    };
    BENCH("Function copy heap", MEDIUM, fp_copy);

    auto std_copy = [&] {
        std::function<int(int,int)> c(ssrc);
        doNotOptimize(c);
    };
    BENCH("std::function copy heap", MEDIUM, std_copy);
}

// Benchmarks moving SBO-stored callables.
static void bench_move_sbo() {
    auto fp_move = [&] {
        Function<int(int,int)> src(SmallCallable{1});
        Function<int(int,int)> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("Function move SBO", MEDIUM, fp_move);

    auto std_move = [&] {
        std::function<int(int,int)> src(SmallCallable{1});
        std::function<int(int,int)> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("std::function move SBO", MEDIUM, std_move);
}

// Benchmarks moving heap-stored callables.
static void bench_move_heap() {
    auto fp_move = [&] {
        Function<int(int,int)> src(LargeCallable{});
        Function<int(int,int)> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("Function move heap", MEDIUM, fp_move);

    auto std_move = [&] {
        std::function<int(int,int)> src(LargeCallable{});
        std::function<int(int,int)> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("std::function move heap", MEDIUM, std_move);
}

// Compares move performance between MoveOnlyFunction and Function.
static void bench_move_only_vs_function() {
    auto mof_move = [&] {
        MoveOnlyFunction<int(int,int)> src(SmallCallable{1});
        MoveOnlyFunction<int(int,int)> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("MoveOnlyFunction move SBO", MEDIUM, mof_move);

    auto fp_move = [&] {
        Function<int(int,int)> src(SmallCallable{1});
        Function<int(int,int)> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("Function move SBO", MEDIUM, fp_move);
}

// Compares invocation overhead across FunctionRef, Function, and std::function.
static void bench_function_ref_vs_function() {
    SmallCallable               callable{1};
    FunctionRef<int(int,int)>   ref(callable);
    Function<int(int,int)>      own(callable);
    std::function<int(int,int)> sf(callable);

    auto ref_invoke = [&] {
        int r = ref(1, 2);
        doNotOptimize(r);
    };
    BENCH("FunctionRef invoke", LARGE, ref_invoke);

    auto fp_invoke = [&] {
        int r = own(1, 2);
        doNotOptimize(r);
    };
    BENCH("Function invoke", LARGE, fp_invoke);

    auto std_invoke = [&] {
        int r = sf(1, 2);
        doNotOptimize(r);
    };
    BENCH("std::function invoke", LARGE, std_invoke);
}

// Executes all Function benchmark cases.
void run_function_benchmarks() {
    setHeader("Function Benchmarks");

    setSubHeader("Construct SBO");
    bench_construct_sbo();
    std::cout << "\n";

    setSubHeader("Construct Heap");
    bench_construct_heap();
    std::cout << "\n";

    setSubHeader("Invoke Free Function");
    bench_invoke_free();
    std::cout << "\n";

    setSubHeader("Invoke SBO");
    bench_invoke_sbo();
    std::cout << "\n";

    setSubHeader("Invoke Heap");
    bench_invoke_heap();
    std::cout << "\n";

    setSubHeader("Copy SBO");
    bench_copy_sbo();
    std::cout << "\n";

    setSubHeader("Copy Heap");
    bench_copy_heap();
    std::cout << "\n";

    setSubHeader("Move SBO");
    bench_move_sbo();
    std::cout << "\n";

    setSubHeader("Move Heap");
    bench_move_heap();
    std::cout << "\n";

    setSubHeader("MoveOnlyFunction vs Function");
    bench_move_only_vs_function();
    std::cout << "\n";

    setSubHeader("FunctionRef vs Function vs std::function");
    bench_function_ref_vs_function();
    borderLine();
    std::cout << "\n";
}