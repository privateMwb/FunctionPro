// FunctionPro operator bool() test suite.
//
// Coverage:
// - Default-constructed instance reports false, for all three types
// - nullptr-constructed instance reports false (Function, MoveOnlyFunction)
// - Bound instance reports true, for all three types
// - After reset(), a previously-bound instance reports false
//   (Function, MoveOnlyFunction — FunctionRef has no reset())

#include <gtest/gtest.h>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

// Verifies a default-constructed Function reports false.
TEST(BoolState, FunctionDefaultIsFalse) {
    Function<int()> f;
    EXPECT_FALSE(static_cast<bool>(f));
}

// Verifies a nullptr-constructed Function reports false.
TEST(BoolState, FunctionNullptrIsFalse) {
    Function<int()> f(nullptr);
    EXPECT_FALSE(static_cast<bool>(f));
}

// Verifies a bound Function reports true.
TEST(BoolState, FunctionBoundIsTrue) {
    Function<int()> f = [] { return 1; };
    EXPECT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), 1); // also exercise invoke() for this binding

    Function<int()> g(f); // also exercise copy() for this binding
    EXPECT_EQ(g(), 1);

    Function<int()> h(std::move(g)); // also exercise move() for this binding
    EXPECT_EQ(h(), 1);
}

// Verifies a Function reports false after reset().
TEST(BoolState, FunctionResetIsFalse) {
    Function<int()> f = [] { return 1; };
    EXPECT_EQ(f(), 1); // exercise invoke() before it's reset away

    Function<int()> g(f); // exercise copy() before it's reset away
    EXPECT_EQ(g(), 1);

    Function<int()> h(std::move(g)); // exercise move() before it's reset away
    EXPECT_EQ(h(), 1);

    f.reset();
    EXPECT_FALSE(static_cast<bool>(f));
}

// Verifies a default-constructed MoveOnlyFunction reports false.
TEST(BoolState, MoveOnlyDefaultIsFalse) {
    MoveOnlyFunction<int()> f;
    EXPECT_FALSE(static_cast<bool>(f));
}

// Verifies a nullptr-constructed MoveOnlyFunction reports false.
TEST(BoolState, MoveOnlyNullptrIsFalse) {
    MoveOnlyFunction<int()> f(nullptr);
    EXPECT_FALSE(static_cast<bool>(f));
}

// Verifies a bound MoveOnlyFunction reports true.
TEST(BoolState, MoveOnlyBoundIsTrue) {
    MoveOnlyFunction<int()> f = [] { return 1; };
    EXPECT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), 1); // also exercise invoke() for this binding

    MoveOnlyFunction<int()> g(std::move(f)); // also exercise move() for this binding
    EXPECT_EQ(g(), 1);
}

// Verifies a MoveOnlyFunction reports false after reset().
TEST(BoolState, MoveOnlyResetIsFalse) {
    MoveOnlyFunction<int()> f = [] { return 1; };
    EXPECT_EQ(f(), 1); // exercise invoke() before it's reset away

    MoveOnlyFunction<int()> g(std::move(f)); // exercise move() for this binding
    EXPECT_EQ(g(), 1);
    f = std::move(g); // move back so f.reset() below still resets a bound instance

    f.reset();
    EXPECT_FALSE(static_cast<bool>(f));
}

// Verifies a default-constructed FunctionRef reports false.
TEST(BoolState, FunctionRefDefaultIsFalse) {
    FunctionRef<int()> f;
    EXPECT_FALSE(static_cast<bool>(f));
}

// Verifies a bound FunctionRef reports true.
TEST(BoolState, FunctionRefBoundIsTrue) {
    auto callable = [] { return 1; };
    FunctionRef<int()> f(callable);
    EXPECT_TRUE(static_cast<bool>(f));
}
