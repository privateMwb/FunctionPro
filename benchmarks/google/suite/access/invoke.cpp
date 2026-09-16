// FunctionPro Invoke Benchmark Suite
// Measures operator() performance for Function, MoveOnlyFunction, and
// FunctionRef against their std counterparts.
//
// Each hit-path case binds a small, SBO-fitting capturing lambda once,
// outside the timed loop — only repeated invocation is measured.
// Invoke cost as capture size grows past the SBO threshold is covered
// separately in scaling/invoke_by_size.cpp.
//
// Covers:
// - operator() hit path, for all three types against their std
//   counterparts (guarded by feature-test macros where needed)
// - operator() on an empty instance — all three throw
//   std::bad_function_call, so this times the throw path itself
// - FunctionRef::operator() bound to a raw function pointer vs bound to
//   a capturing lambda — FunctionRef special-cases function pointers
//   internally to skip an extra indirection, so this isolates that path

#include <benchmark/benchmark.h>

#include <functional>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

namespace {
constexpr int kAddend = 7;

int addSeven(int x) {
    return x + kAddend;
}
} // namespace

// Measures Function::operator(), hit path.
static void Function_Invoke(benchmark::State& state) {
    int captured = kAddend;
    Function<int(int)> cSrc = [captured](int x) { return x + captured; };

    for (auto _ : state) {
        int v = cSrc(1);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(Function_Invoke);

// Measures std::function::operator(), hit path.
static void StdFunction_Invoke(benchmark::State& state) {
    int captured = kAddend;
    std::function<int(int)> sSrc = [captured](int x) { return x + captured; };

    for (auto _ : state) {
        int v = sSrc(1);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(StdFunction_Invoke);

// Measures Function::operator() on an empty instance — throws
// std::bad_function_call.
static void Function_Invoke_Empty(benchmark::State& state) {
    Function<int(int)> cSrc;

    for (auto _ : state) {
        try {
            int v = cSrc(1);
            benchmark::DoNotOptimize(v);
        } catch (const std::bad_function_call&) {
        }
    }
}
BENCHMARK(Function_Invoke_Empty);

// Measures std::function::operator() on an empty instance — throws
// std::bad_function_call.
static void StdFunction_Invoke_Empty(benchmark::State& state) {
    std::function<int(int)> sSrc;

    for (auto _ : state) {
        try {
            int v = sSrc(1);
            benchmark::DoNotOptimize(v);
        } catch (const std::bad_function_call&) {
        }
    }
}
BENCHMARK(StdFunction_Invoke_Empty);

// Measures MoveOnlyFunction::operator(), hit path.
static void MoveOnlyFunction_Invoke(benchmark::State& state) {
    int captured = kAddend;
    MoveOnlyFunction<int(int)> cSrc = [captured](int x) { return x + captured; };

    for (auto _ : state) {
        int v = cSrc(1);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(MoveOnlyFunction_Invoke);

#if defined(__cpp_lib_move_only_function)
// Measures std::move_only_function::operator(), hit path, where
// available.
static void StdMoveOnlyFunction_Invoke(benchmark::State& state) {
    int captured = kAddend;
    std::move_only_function<int(int)> sSrc = [captured](int x) { return x + captured; };

    for (auto _ : state) {
        int v = sSrc(1);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(StdMoveOnlyFunction_Invoke);
#endif

// Measures MoveOnlyFunction::operator() on an empty instance.
// No std::move_only_function comparison here: unlike std::function,
// invoking an empty std::move_only_function is undefined behavior
// (not guaranteed to throw), so there's no safe reference-side case.
static void MoveOnlyFunction_Invoke_Empty(benchmark::State& state) {
    MoveOnlyFunction<int(int)> cSrc;

    for (auto _ : state) {
        try {
            int v = cSrc(1);
            benchmark::DoNotOptimize(v);
        } catch (const std::bad_function_call&) {
        }
    }
}
BENCHMARK(MoveOnlyFunction_Invoke_Empty);

// Measures FunctionRef::operator(), hit path, bound to a capturing
// lambda.
static void FunctionRef_Invoke(benchmark::State& state) {
    int captured = kAddend;
    auto callable = [captured](int x) { return x + captured; };
    FunctionRef<int(int)> cSrc(callable);

    for (auto _ : state) {
        int v = cSrc(1);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(FunctionRef_Invoke);

#if defined(__cpp_lib_function_ref)
// Measures std::function_ref::operator(), hit path, bound to a
// capturing lambda, where available.
static void StdFunctionRef_Invoke(benchmark::State& state) {
    int captured = kAddend;
    auto callable = [captured](int x) { return x + captured; };
    std::function_ref<int(int)> sSrc(callable);

    for (auto _ : state) {
        int v = sSrc(1);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(StdFunctionRef_Invoke);
#endif

// Measures FunctionRef::operator() on an empty instance.
static void FunctionRef_Invoke_Empty(benchmark::State& state) {
    FunctionRef<int(int)> cSrc;

    for (auto _ : state) {
        try {
            int v = cSrc(1);
            benchmark::DoNotOptimize(v);
        } catch (const std::bad_function_call&) {
        }
    }
}
BENCHMARK(FunctionRef_Invoke_Empty);

#if defined(__cpp_lib_function_ref)
// Measures std::function_ref::operator() on an empty instance, where
// available.
static void StdFunctionRef_Invoke_Empty(benchmark::State& state) {
    std::function_ref<int(int)> sSrc;

    for (auto _ : state) {
        try {
            int v = sSrc(1);
            benchmark::DoNotOptimize(v);
        } catch (const std::bad_function_call&) {
        }
    }
}
BENCHMARK(StdFunctionRef_Invoke_Empty);
#endif

// Measures FunctionRef::operator() bound to a raw function pointer,
// isolating the function-pointer fast path FunctionRef special-cases
// internally to skip storing (and indirecting through) an object
// pointer.
static void FunctionRef_Invoke_FnPtr(benchmark::State& state) {
    FunctionRef<int(int)> cSrc(addSeven);

    for (auto _ : state) {
        int v = cSrc(1);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(FunctionRef_Invoke_FnPtr);

#if defined(__cpp_lib_function_ref)
// Measures std::function_ref::operator() bound to a raw function
// pointer, where available.
static void StdFunctionRef_Invoke_FnPtr(benchmark::State& state) {
    std::function_ref<int(int)> sSrc(addSeven);

    for (auto _ : state) {
        int v = sSrc(1);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(StdFunctionRef_Invoke_FnPtr);
#endif
