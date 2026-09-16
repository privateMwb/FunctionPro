// FunctionPro copy constructor test suite.
//
// Coverage:
// - Function copy construct produces a deep, independent copy -- for
//   both SBO-stored and heap-stored callables -- verified via a
//   captured shared_ptr's use_count increasing on copy and each
//   instance invoking independently
// - MoveOnlyFunction is excluded entirely: its copy constructor is
//   deleted (matching std::move_only_function), there is nothing to
//   test
// - FunctionRef copy construct is the contrasting case: it produces a
//   shallow copy that references the *same* external object, not an
//   independent one -- mutating the referenced object is visible
//   through both the original and the copy identically

#include <gtest/gtest.h>

#include <array>
#include <memory>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>

using namespace FunctionPro;

namespace {
// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit.
struct LargePayload {
    std::shared_ptr<int> tracked;
    std::array<std::byte, 64> padding{};
    int operator()() const {
        return *tracked;
    }
};

struct Multiplier {
    int factor;
    int operator()(int x) const {
        return x * factor;
    }
};
} // namespace

// Verifies copying a Function holding an SBO-stored callable produces
// an independent copy.
TEST(CopyConstructor, CopySboIndependent) {
    auto tracked = std::make_shared<int>(5);
    Function<int()> src = [tracked] { return *tracked; };
    EXPECT_EQ(tracked.use_count(), 2);

    Function<int()> dst(src);
    EXPECT_EQ(tracked.use_count(), 3); // dst holds its own captured copy

    EXPECT_EQ(src(), 5);
    EXPECT_EQ(dst(), 5);

    Function<int()> moved(std::move(dst)); // exercise move() for this binding
    EXPECT_EQ(moved(), 5);
}

// Verifies copying a Function holding a heap-stored callable produces
// an independent copy, with independent heap storage.
TEST(CopyConstructor, CopyHeapIndependent) {
    auto tracked = std::make_shared<int>(9);
    Function<int()> src = LargePayload{tracked, {}};
    EXPECT_EQ(tracked.use_count(), 2);

    Function<int()> dst(src);
    EXPECT_EQ(tracked.use_count(), 3); // dst holds its own captured copy

    EXPECT_EQ(src(), 9);
    EXPECT_EQ(dst(), 9);

    src = [] { return -1; }; // reassign original, must not affect dst
    EXPECT_EQ(dst(), 9);
    EXPECT_EQ(src(), -1); // exercise invoke() for the new binding

    Function<int()> other(src); // exercise copy() for the new binding
    EXPECT_EQ(other(), -1);
}

// Verifies copying a FunctionRef produces a shallow copy referencing
// the same external object -- the opposite of Function's independence.
TEST(CopyConstructor, RefCopySharesObject) {
    Multiplier m{2};
    FunctionRef<int(int)> src(m);
    FunctionRef<int(int)> dst(src);

    EXPECT_EQ(src(5), 10);
    EXPECT_EQ(dst(5), 10);

    m.factor = 10;         // mutate the external object
    EXPECT_EQ(src(5), 50); // both reflect the change identically...
    EXPECT_EQ(dst(5), 50); // ...since both reference the same object
}
