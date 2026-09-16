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

#include <gtest/gtest.h>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

// Verifies a bound Function returns the callable's result.
TEST(Invocation, FunctionInvokeReturnsResult) {
    Function<int()> f = [] { return 42; };
    EXPECT_EQ(f(), 42);

    Function<int()> g(f); // exercise copy() for this binding
    EXPECT_EQ(g(), 42);

    Function<int()> h(std::move(g)); // exercise move() for this binding
    EXPECT_EQ(h(), 42);
}

// Verifies arguments are forwarded correctly to the stored callable.
TEST(Invocation, FunctionInvokeForwardsArguments) {
    Function<int(int, int)> f = [](int a, int b) { return a * 10 + b; };
    EXPECT_EQ(f(3, 7), 37);

    Function<int(int, int)> g(f); // exercise copy() for this binding
    EXPECT_EQ(g(3, 7), 37);

    Function<int(int, int)> h(std::move(g)); // exercise move() for this binding
    EXPECT_EQ(h(3, 7), 37);
}

// Verifies operator() is callable through a const Function reference.
TEST(Invocation, FunctionInvokeViaConstRef) {
    const Function<int()> f = [] { return 5; };
    EXPECT_EQ(f(), 5);

    Function<int()> g(f); // exercise copy() from a const source
    EXPECT_EQ(g(), 5);

    const Function<int()> h(std::move(g)); // exercise move() for this binding
    EXPECT_EQ(h(), 5);
}

// Verifies calling an empty Function throws std::bad_function_call.
TEST(Invocation, FunctionInvokeEmptyThrows) {
    Function<int()> f;
    EXPECT_THROW(f(), std::bad_function_call);
}

// Verifies a bound MoveOnlyFunction returns the callable's result.
TEST(Invocation, MoveOnlyInvokeReturnsResult) {
    MoveOnlyFunction<int()> f = [] { return 42; };
    EXPECT_EQ(f(), 42);

    MoveOnlyFunction<int()> g(std::move(f)); // exercise move() for this binding
    EXPECT_EQ(g(), 42);
}

// Verifies arguments are forwarded correctly to the stored callable.
TEST(Invocation, MoveOnlyInvokeForwardsArguments) {
    MoveOnlyFunction<int(int, int)> f = [](int a, int b) { return a * 10 + b; };
    EXPECT_EQ(f(3, 7), 37);

    MoveOnlyFunction<int(int, int)> g(std::move(f)); // exercise move()
    EXPECT_EQ(g(3, 7), 37);
}

// Verifies calling an empty MoveOnlyFunction throws std::bad_function_call.
TEST(Invocation, MoveOnlyInvokeEmptyThrows) {
    MoveOnlyFunction<int()> f;
    EXPECT_THROW(f(), std::bad_function_call);
}

// Verifies a bound FunctionRef returns the referenced callable's result.
TEST(Invocation, FunctionRefInvokeReturnsResult) {
    auto callable = [] { return 42; };
    FunctionRef<int()> f(callable);
    EXPECT_EQ(f(), 42);
}

// Verifies arguments are forwarded correctly through FunctionRef.
TEST(Invocation, FunctionRefInvokeForwardsArguments) {
    auto callable = [](int a, int b) { return a * 10 + b; };
    FunctionRef<int(int, int)> f(callable);
    EXPECT_EQ(f(3, 7), 37);
}

// Verifies calling an empty FunctionRef throws std::bad_function_call.
TEST(Invocation, FunctionRefInvokeEmptyThrows) {
    FunctionRef<int()> f;
    EXPECT_THROW(f(), std::bad_function_call);
}
