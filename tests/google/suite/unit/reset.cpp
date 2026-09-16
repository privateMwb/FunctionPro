// FunctionPro reset() test suite.
//
// Coverage:
// - After reset(), the instance reports empty (operator bool() is
//   false) and invoking it throws std::bad_function_call
// - reset() on an already-empty instance is a safe no-op
// - reset() actually destroys the stored callable's resources (verified
//   via a shared_ptr use_count, not just that the API reports empty)
// - FunctionRef has no reset() and is not covered here

#include <gtest/gtest.h>

#include <memory>

#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

// Verifies Function::reset() leaves the instance empty and unusable.
TEST(Reset, FunctionResetLeavesEmpty) {
    Function<int()> f = [] { return 1; };
    EXPECT_EQ(f(), 1); // exercise invoke() while still bound

    Function<int()> g(f); // exercise copy() while still bound
    EXPECT_EQ(g(), 1);

    Function<int()> h(std::move(g)); // exercise move() while still bound
    EXPECT_EQ(h(), 1);

    f.reset();

    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_THROW(f(), std::bad_function_call);
}

// Verifies Function::reset() on an already-empty instance is a safe no-op.
TEST(Reset, FunctionResetOnEmptyIsNoop) {
    Function<int()> f;
    f.reset();
    EXPECT_FALSE(static_cast<bool>(f));

    f.reset(); // second call, still nothing to release
    EXPECT_FALSE(static_cast<bool>(f));
}

// Verifies Function::reset() actually releases the stored callable's
// resources, not just that the API reports empty afterward.
TEST(Reset, FunctionResetReleasesResources) {
    auto tracked = std::make_shared<int>(1);
    EXPECT_EQ(tracked.use_count(), 1);

    Function<int()> f = [tracked] { return *tracked; };
    EXPECT_EQ(tracked.use_count(), 2); // captured copy inside f
    EXPECT_EQ(f(), 1);                 // exercise invoke() while still bound

    {
        Function<int()> g(f); // exercise copy() while still bound
        EXPECT_EQ(tracked.use_count(), 3);
        EXPECT_EQ(g(), 1);

        Function<int()> h(std::move(g));   // exercise move() while still bound
        EXPECT_EQ(tracked.use_count(), 3); // move transfers ownership, adds no reference
        EXPECT_EQ(h(), 1);
    } // g (moved-from) and h destroyed here, back to baseline before reset() below

    f.reset();
    EXPECT_EQ(tracked.use_count(), 1); // f's copy was destroyed
}

// Verifies MoveOnlyFunction::reset() leaves the instance empty and
// unusable.
TEST(Reset, MoveOnlyResetLeavesEmpty) {
    MoveOnlyFunction<int()> f = [] { return 1; };
    EXPECT_EQ(f(), 1); // exercise invoke() while still bound

    MoveOnlyFunction<int()> g(std::move(f)); // exercise move() while still bound
    EXPECT_EQ(g(), 1);
    f = std::move(g); // move back so f.reset() below still resets a bound instance

    f.reset();

    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_THROW(f(), std::bad_function_call);
}

// Verifies MoveOnlyFunction::reset() on an already-empty instance is a
// safe no-op.
TEST(Reset, MoveOnlyResetOnEmptyIsNoop) {
    MoveOnlyFunction<int()> f;
    f.reset();
    EXPECT_FALSE(static_cast<bool>(f));

    f.reset();
    EXPECT_FALSE(static_cast<bool>(f));
}

// Verifies MoveOnlyFunction::reset() actually releases the stored
// callable's resources.
TEST(Reset, MoveOnlyResetReleasesResources) {
    auto tracked = std::make_shared<int>(1);
    EXPECT_EQ(tracked.use_count(), 1);

    MoveOnlyFunction<int()> f = [tracked] { return *tracked; };
    EXPECT_EQ(tracked.use_count(), 2);
    EXPECT_EQ(f(), 1); // exercise invoke() while still bound

    {
        MoveOnlyFunction<int()> g(std::move(f)); // exercise move() while still bound
        EXPECT_EQ(tracked.use_count(), 2);       // move transfers ownership, adds no reference
        EXPECT_EQ(g(), 1);
        f = std::move(g); // move back so the checks below are unaffected
    }

    f.reset();
    EXPECT_EQ(tracked.use_count(), 1);
}
