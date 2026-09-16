// FunctionPro operator() test suite.
//
// Coverage:
// - Bound instance invokes the stored callable and returns its result,
//   for all three types
// - Arguments are forwarded correctly (multiple args, non-trivial types)
// - Calling an empty instance throws std::bad_function_call, for all
//   three types
// - Function's operator() is const-callable (can be invoked through a
//   const reference)

#include <support/framework.h>

using namespace FunctionPro;

// Verifies a bound Function returns the callable's result.
static void function_invoke_returns_result() {
    Function<int()> f = [] { return 42; };
    CHK(f() == 42);

    Function<int()> g(f); // exercise copy() for this binding
    CHK(g() == 42);

    Function<int()> h(std::move(g)); // exercise move() for this binding
    CHK(h() == 42);
}

// Verifies arguments are forwarded correctly to the stored callable.
static void function_invoke_forwards_arguments() {
    Function<int(int, int)> f = [](int a, int b) { return a * 10 + b; };
    CHK(f(3, 7) == 37);

    Function<int(int, int)> g(f); // exercise copy() for this binding
    CHK(g(3, 7) == 37);

    Function<int(int, int)> h(std::move(g)); // exercise move() for this binding
    CHK(h(3, 7) == 37);
}

// Verifies operator() is callable through a const Function reference.
static void function_invoke_via_const_ref() {
    const Function<int()> f = [] { return 5; };
    CHK(f() == 5);

    Function<int()> g(f); // exercise copy() from a const source
    CHK(g() == 5);

    const Function<int()> h(std::move(g)); // exercise move() for this binding
    CHK(h() == 5);
}

// Verifies calling an empty Function throws std::bad_function_call.
static void function_invoke_empty_throws() {
    Function<int()> f;
    CHK_THROWS(f(), std::bad_function_call);
}

// Verifies a bound MoveOnlyFunction returns the callable's result.
static void move_only_invoke_returns_result() {
    MoveOnlyFunction<int()> f = [] { return 42; };
    CHK(f() == 42);

    MoveOnlyFunction<int()> g(std::move(f)); // exercise move() for this binding
    CHK(g() == 42);
}

// Verifies arguments are forwarded correctly to the stored callable.
static void move_only_invoke_forwards_arguments() {
    MoveOnlyFunction<int(int, int)> f = [](int a, int b) { return a * 10 + b; };
    CHK(f(3, 7) == 37);

    MoveOnlyFunction<int(int, int)> g(std::move(f)); // exercise move()
    CHK(g(3, 7) == 37);
}

// Verifies calling an empty MoveOnlyFunction throws std::bad_function_call.
static void move_only_invoke_empty_throws() {
    MoveOnlyFunction<int()> f;
    CHK_THROWS(f(), std::bad_function_call);
}

// Verifies a bound FunctionRef returns the referenced callable's result.
static void function_ref_invoke_returns_result() {
    auto callable = [] { return 42; };
    FunctionRef<int()> f(callable);
    CHK(f() == 42);
}

// Verifies arguments are forwarded correctly through FunctionRef.
static void function_ref_invoke_forwards_arguments() {
    auto callable = [](int a, int b) { return a * 10 + b; };
    FunctionRef<int(int, int)> f(callable);
    CHK(f(3, 7) == 37);
}

// Verifies calling an empty FunctionRef throws std::bad_function_call.
static void function_ref_invoke_empty_throws() {
    FunctionRef<int()> f;
    CHK_THROWS(f(), std::bad_function_call);
}

// Executes all operator() test cases.
static void run_tests() {
    RUN(function_invoke_returns_result);
    RUN(function_invoke_forwards_arguments);
    RUN(function_invoke_via_const_ref);
    RUN(function_invoke_empty_throws);

    RUN(move_only_invoke_returns_result);
    RUN(move_only_invoke_forwards_arguments);
    RUN(move_only_invoke_empty_throws);

    RUN(function_ref_invoke_returns_result);
    RUN(function_ref_invoke_forwards_arguments);
    RUN(function_ref_invoke_empty_throws);
}

REGISTER_TEST_SUITE();
