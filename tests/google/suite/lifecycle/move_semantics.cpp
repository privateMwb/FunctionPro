// FunctionPro move constructor test suite.
//
// Coverage:
// - Function/MoveOnlyFunction move construct, for both SBO-stored and
//   heap-stored callables: destination invokes correctly, source is
//   left empty (operator bool() is false) -- both types explicitly null
//   the source's vtable pointer as part of their custom move
// - FunctionRef move construct is the contrasting case: its move
//   constructor is defaulted (a trivial memberwise copy of the pointer
//   pair), so the source is NOT left empty -- it still references the
//   same external object after being "moved from". This is real,
//   observable behavior worth documenting explicitly rather than
//   assuming it matches the other two types' contract.

#include <gtest/gtest.h>

#include <array>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

namespace {
constexpr int kPad = 3; // keeps the small lambda's capture well under 40 bytes

// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit.
struct LargePayload {
    std::array<std::byte, 64> padding{};
    int tag;
    int operator()() const {
        return tag;
    }
};
} // namespace

// Verifies move-constructing a Function from an SBO-stored callable
// transfers correctly and leaves the source empty.
TEST(MoveSemantics, MoveSboEmptiesSource) {
    int a = kPad, b = kPad, c = kPad;
    Function<int()> src = [a, b, c] { return a + b + c; };

    Function<int()> dst(std::move(src));

    EXPECT_EQ(dst(), 9);
    EXPECT_FALSE(static_cast<bool>(src));

    Function<int()> other(dst); // exercise copy() for this binding
    EXPECT_EQ(other(), 9);
}

// Verifies move-constructing a Function from a heap-stored callable
// transfers correctly and leaves the source empty.
TEST(MoveSemantics, MoveHeapEmptiesSource) {
    Function<int()> src = LargePayload{{}, 42};

    Function<int()> dst(std::move(src));

    EXPECT_EQ(dst(), 42);
    EXPECT_FALSE(static_cast<bool>(src));
}

// Verifies move-constructing a MoveOnlyFunction from an SBO-stored
// callable transfers correctly and leaves the source empty.
TEST(MoveSemantics, MoveOnlySboEmptiesSource) {
    int a = kPad, b = kPad, c = kPad;
    MoveOnlyFunction<int()> src = [a, b, c] { return a + b + c; };

    MoveOnlyFunction<int()> dst(std::move(src));

    EXPECT_EQ(dst(), 9);
    EXPECT_FALSE(static_cast<bool>(src));
}

// Verifies move-constructing a MoveOnlyFunction from a heap-stored
// callable transfers correctly and leaves the source empty.
TEST(MoveSemantics, MoveOnlyHeapEmptiesSource) {
    MoveOnlyFunction<int()> src = LargePayload{{}, 42};

    MoveOnlyFunction<int()> dst(std::move(src));

    EXPECT_EQ(dst(), 42);
    EXPECT_FALSE(static_cast<bool>(src));
}

// Verifies move-constructing a FunctionRef transfers correctly to the
// destination -- and, unlike the other two types, does NOT leave the
// source empty, since FunctionRef's move constructor is a defaulted
// trivial copy rather than a custom nulling move.
TEST(MoveSemantics, RefMoveKeepsSource) {
    auto callable = [] { return 7; };
    FunctionRef<int()> src(callable);

    FunctionRef<int()> dst(std::move(src));

    EXPECT_EQ(dst(), 7);
    EXPECT_TRUE(static_cast<bool>(src)); // still true -- src was not nulled
    EXPECT_EQ(src(), 7);                 // still fully usable, references the same object
}
