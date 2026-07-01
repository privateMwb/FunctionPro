// MoveOnlyFunction Benchmark Suite
// Compares MoveOnlyFunction against Function across construction,
// invocation, move, and reset operations, including move-only callables.
//
// Covers:
// - SBO construction
// - heap construction
// - move-only callable construction
// - free function invocation
// - SBO invocation
// - heap invocation
// - move-only callable invocation
// - SBO move
// - heap move
// - unique_ptr capture move
// - SBO reset
// - heap reset

#include "bench_helper.h"

#include <functional>
#include <memory>

using namespace FunctionPro;

// Free function used by invocation benchmarks.
static int free_add(int a, int b) { return a + b; }

// Small callable that fits within the small-buffer optimization.
struct SmallCallable {
    int bias;
    int operator()(int a, int b) const { return a + b + bias; }
};

// Large callable that forces heap allocation.
struct LargeCallable {
    std::byte pad[64] = {};
    int operator()(int a, int b) const { return a + b; }
};

// Benchmarks SBO construction.
static void bench_construct_sbo() {
    auto mof_sbo = [&] {
        MoveOnlyFunction<int(int,int)> f(SmallCallable{1});
        doNotOptimize(f);
    };
    BENCH("MoveOnlyFunction construct SBO", MEDIUM, mof_sbo);

    auto fp_sbo = [&] {
        Function<int(int,int)> f(SmallCallable{1});
        doNotOptimize(f);
    };
    BENCH("Function construct SBO", MEDIUM, fp_sbo);
}

// Benchmarks heap construction.
static void bench_construct_heap() {
    auto mof_heap = [&] {
        MoveOnlyFunction<int(int,int)> f(LargeCallable{});
        doNotOptimize(f);
    };
    BENCH("MoveOnlyFunction construct heap", MEDIUM, mof_heap);

    auto fp_heap = [&] {
        Function<int(int,int)> f(LargeCallable{});
        doNotOptimize(f);
    };
    BENCH("Function construct heap", MEDIUM, fp_heap);
}

// Benchmarks construction from a move-only callable.
static void bench_construct_move_only_callable() {
    auto mof_unique = [&] {
        MoveOnlyFunction<int()> f([p = std::make_unique<int>(42)]() { return *p; });
        doNotOptimize(f);
    };
    BENCH("construct unique_ptr capture", MEDIUM, mof_unique);
}

// Benchmarks free function invocation.
static void bench_invoke_free() {
    MoveOnlyFunction<int(int,int)> mof(free_add);
    Function<int(int,int)>         fp(free_add);

    auto mof_invoke = [&] {
        int r = mof(1, 2);
        doNotOptimize(r);
    };
    BENCH("MoveOnlyFunction invoke free", LARGE, mof_invoke);

    auto fp_invoke = [&] {
        int r = fp(1, 2);
        doNotOptimize(r);
    };
    BENCH("Function invoke free", LARGE, fp_invoke);
}

// Benchmarks SBO callable invocation.
static void bench_invoke_sbo() {
    MoveOnlyFunction<int(int,int)> mof(SmallCallable{1});
    Function<int(int,int)>         fp(SmallCallable{1});

    auto mof_invoke = [&] {
        int r = mof(1, 2);
        doNotOptimize(r);
    };
    BENCH("MoveOnlyFunction invoke SBO", LARGE, mof_invoke);

    auto fp_invoke = [&] {
        int r = fp(1, 2);
        doNotOptimize(r);
    };
    BENCH("Function invoke SBO", LARGE, fp_invoke);
}

// Benchmarks heap callable invocation.
static void bench_invoke_heap() {
    MoveOnlyFunction<int(int,int)> mof(LargeCallable{});
    Function<int(int,int)>         fp(LargeCallable{});

    auto mof_invoke = [&] {
        int r = mof(1, 2);
        doNotOptimize(r);
    };
    BENCH("MoveOnlyFunction invoke heap", LARGE, mof_invoke);

    auto fp_invoke = [&] {
        int r = fp(1, 2);
        doNotOptimize(r);
    };
    BENCH("Function invoke heap", LARGE, fp_invoke);
}

// Benchmarks invocation of a move-only callable.
static void bench_invoke_move_only_callable() {
    int val = 42;
    MoveOnlyFunction<int()> mof([&val]() { return val; });

    auto mof_invoke = [&] {
        int r = mof();
        doNotOptimize(r);
    };
    BENCH("invoke move-only capture", LARGE, mof_invoke);
}

// Benchmarks SBO move construction.
static void bench_move_sbo() {
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

// Benchmarks heap move construction.
static void bench_move_heap() {
    auto mof_move = [&] {
        MoveOnlyFunction<int(int,int)> src(LargeCallable{});
        MoveOnlyFunction<int(int,int)> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("MoveOnlyFunction move heap", MEDIUM, mof_move);

    auto fp_move = [&] {
        Function<int(int,int)> src(LargeCallable{});
        Function<int(int,int)> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("Function move heap", MEDIUM, fp_move);
}

// Benchmarks moving a callable that owns a unique_ptr.
static void bench_move_unique_ptr_capture() {
    auto mof_move = [&] {
        MoveOnlyFunction<int()> src([p = std::make_unique<int>(99)]() { return *p; });
        MoveOnlyFunction<int()> dst(std::move(src));
        doNotOptimize(dst);
    };
    BENCH("move unique_ptr capture", MEDIUM, mof_move);
}

// Benchmarks reset after SBO storage.
static void bench_reset_sbo() {
    auto mof_reset = [&] {
        MoveOnlyFunction<int(int,int)> f(SmallCallable{1});
        f.reset();
        doNotOptimize(f);
    };
    BENCH("MoveOnlyFunction reset SBO", MEDIUM, mof_reset);

    auto fp_reset = [&] {
        Function<int(int,int)> f(SmallCallable{1});
        f.reset();
        doNotOptimize(f);
    };
    BENCH("Function reset SBO", MEDIUM, fp_reset);
}

// Benchmarks reset after heap storage.
static void bench_reset_heap() {
    auto mof_reset = [&] {
        MoveOnlyFunction<int(int,int)> f(LargeCallable{});
        f.reset();
        doNotOptimize(f);
    };
    BENCH("MoveOnlyFunction reset heap", MEDIUM, mof_reset);

    auto fp_reset = [&] {
        Function<int(int,int)> f(LargeCallable{});
        f.reset();
        doNotOptimize(f);
    };
    BENCH("Function reset heap", MEDIUM, fp_reset);
}

// Executes all MoveOnlyFunction benchmarks.
void run_move_only_function_benchmarks() {
    setHeader("MoveOnlyFunction Benchmarks");

    setSubHeader("Construct SBO");
    bench_construct_sbo();
    std::cout << "\n";

    setSubHeader("Construct Heap");
    bench_construct_heap();
    std::cout << "\n";

    setSubHeader("Construct Move-Only Callable");
    bench_construct_move_only_callable();
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

    setSubHeader("Invoke Move-Only Callable");
    bench_invoke_move_only_callable();
    std::cout << "\n";

    setSubHeader("Move SBO");
    bench_move_sbo();
    std::cout << "\n";

    setSubHeader("Move Heap");
    bench_move_heap();
    std::cout << "\n";

    setSubHeader("Move Unique Ptr Capture");
    bench_move_unique_ptr_capture();
    std::cout << "\n";

    setSubHeader("Reset SBO");
    bench_reset_sbo();
    std::cout << "\n";

    setSubHeader("Reset Heap");
    bench_reset_heap();
    borderLine();
    std::cout << "\n";
}