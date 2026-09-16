// FunctionPro moved-from state regression test suite.
//
// Not a historical bug -- confirmed safe during audit, kept here as a
// guard against a future change silently breaking the guarantee.
//
// lifecycle/move_semantics.cpp verifies a moved-from instance reports
// empty. This file goes further: it verifies a moved-from instance is
// genuinely *safe to keep using*, not just empty by coincidence --
// calling it throws cleanly rather than reading garbage state,
// reset() on it is a safe no-op, and assigning a new callable into it
// afterward fully revives it rather than leaving it in some
// half-broken state.
//
// Coverage:
// - Function/MoveOnlyFunction: calling a moved-from instance throws
//   std::bad_function_call, reset() on it is a safe no-op, and it can
//   be reassigned afterward to become fully usable again
// - FunctionRef: not covered here -- its move constructor doesn't
//   empty the source at all (see lifecycle/move_semantics.cpp), so
//   there's no moved-from state to guard

#include <gtest/gtest.h>

#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

// Verifies a moved-from Function is safe to call (throws cleanly), safe
// to reset() (no-op), and can be fully revived by reassignment.
TEST(MovedFromState, MovedFromSafeReusable) {
    Function<int()> src = [] { return 1; };
    Function<int()> dst(std::move(src));

    EXPECT_THROW(src(), std::bad_function_call);

    src.reset(); // must be a safe no-op, not a double-release
    EXPECT_FALSE(static_cast<bool>(src));

    src = [] { return 2; }; // revive it
    EXPECT_TRUE(static_cast<bool>(src));
    EXPECT_EQ(src(), 2);
    EXPECT_EQ(dst(), 1); // dst remains unaffected throughout

    Function<int()> dstCopy(dst); // exercise copy() of dst's original binding
    EXPECT_EQ(dstCopy(), 1);

    Function<int()> srcCopy(src); // exercise copy() of src's revived binding
    EXPECT_EQ(srcCopy(), 2);
}

// Verifies the same for move-assignment as the source of the move,
// not just move-construction.
TEST(MovedFromState, MovedViaAssignSafe) {
    Function<int()> src = [] { return 1; };
    Function<int()> dst;
    dst = std::move(src);

    EXPECT_THROW(src(), std::bad_function_call);

    src.reset();
    EXPECT_FALSE(static_cast<bool>(src));

    src = [] { return 3; };
    EXPECT_EQ(src(), 3);
    EXPECT_EQ(dst(), 1);

    Function<int()> dstCopy(dst); // exercise copy() of dst's original binding
    EXPECT_EQ(dstCopy(), 1);

    Function<int()> srcCopy(src); // exercise copy() of src's revived binding
    EXPECT_EQ(srcCopy(), 3);
}

// Verifies a moved-from MoveOnlyFunction is safe to call (throws
// cleanly), safe to reset() (no-op), and can be fully revived by
// reassignment.
TEST(MovedFromState, MoveOnlyMovedSafeReusable) {
    MoveOnlyFunction<int()> src = [] { return 1; };
    MoveOnlyFunction<int()> dst(std::move(src));

    EXPECT_THROW(src(), std::bad_function_call);

    src.reset();
    EXPECT_FALSE(static_cast<bool>(src));

    src = [] { return 2; };
    EXPECT_TRUE(static_cast<bool>(src));
    EXPECT_EQ(src(), 2);
    EXPECT_EQ(dst(), 1);
}
