// FunctionPro copy-assign flow integration test suite.
//
// Coverage:
// - Copy-assign into an already-bound Function correctly releases the
//   old callable's resources before taking on the new one -- a real
//   end-to-end flow (destroy-old + construct-new via assignment), not
//   just copy-construction into a fresh instance
// - The copy remains independent of the source after assignment, same
//   as copy_constructor.cpp verifies for construction
// - MoveOnlyFunction is excluded: copy-assign is deleted, nothing to
//   test
// - FunctionRef copy-assign rebinds to a different referenced object
//   cleanly -- the old reference is simply dropped (nothing to
//   release, it never owned it)

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

// Verifies copy-assigning a new SBO-fitting callable over an
// already-bound Function replaces it correctly, and the copy is
// independent of the source.
TEST(CopyAssignFlow, ReplacesExistingBinding) {
    auto tracked = std::make_shared<int>(3);
    Function<int()> dst = [] { return -1; }; // starts bound to something else
    EXPECT_EQ(dst(), -1);                    // exercise invoke() for dst's initial binding

    {
        Function<int()> dstCopy(dst); // exercise copy() for dst's initial binding
        EXPECT_EQ(dstCopy(), -1);

        Function<int()> dstMoved(std::move(dstCopy)); // exercise move()
        EXPECT_EQ(dstMoved(), -1);
    } // dstCopy (moved-from) and dstMoved destroyed here, doesn't affect anything below

    Function<int()> src = [tracked] { return *tracked; };
    EXPECT_EQ(tracked.use_count(), 2);

    dst = src;

    EXPECT_EQ(tracked.use_count(), 3); // dst now holds its own captured copy
    EXPECT_EQ(dst(), 3);
    EXPECT_EQ(src(), 3);
}

// Verifies copy-assigning over a Function that previously held a
// heap-stored callable correctly releases the old heap resource before
// taking on the new binding.
TEST(CopyAssignFlow, ReleasesOldHeap) {
    auto oldTracked = std::make_shared<int>(1);
    auto newTracked = std::make_shared<int>(2);

    Function<int()> dst = LargePayload{oldTracked, {}};
    EXPECT_EQ(oldTracked.use_count(), 2);

    Function<int()> src = LargePayload{newTracked, {}};
    dst = src; // old heap-stored callable must be released here

    EXPECT_EQ(oldTracked.use_count(), 1); // released
    EXPECT_EQ(newTracked.use_count(), 3); // dst + src each hold a copy
    EXPECT_EQ(dst(), 2);
    EXPECT_EQ(src(), 2);
}

// Verifies copy-assigning a FunctionRef rebinds it to a different
// referenced object cleanly.
TEST(CopyAssignFlow, RefRebindsReferent) {
    Multiplier a{2};
    Multiplier b{5};
    FunctionRef<int(int)> dst(a);
    FunctionRef<int(int)> src(b);

    EXPECT_EQ(dst(3), 6);

    dst = src;

    EXPECT_EQ(dst(3), 15); // now references b
    a.factor = 100;        // mutating the old referent no longer affects dst
    EXPECT_EQ(dst(3), 15);
}
