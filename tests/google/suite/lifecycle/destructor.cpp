// FunctionPro destructor test suite.
//
// Coverage:
// - Destroying a Function/MoveOnlyFunction releases the stored
//   callable's own resources (verified via a captured shared_ptr's
//   use_count), for both SBO-stored and heap-stored callables
// - Destroying an empty instance is safe (no crash, no-op) -- relevant
//   since the vtable pointer is what gates whether destroy() runs at
//   all
// - Destroying a FunctionRef does NOT affect the referenced external
//   object -- it never owned it in the first place, so the referenced
//   object's resources are completely unaffected by the FunctionRef's
//   own destruction

#include <gtest/gtest.h>

#include <array>
#include <memory>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

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
} // namespace

// Verifies destroying a Function holding an SBO-stored callable
// releases that callable's own resources.
TEST(Destructor, DestructorReleasesSbo) {
    auto tracked = std::make_shared<int>(1);
    EXPECT_EQ(tracked.use_count(), 1);
    {
        Function<int()> f = [tracked] { return *tracked; };
        EXPECT_EQ(tracked.use_count(), 2);
        EXPECT_EQ(f(), 1); // exercise invoke() before destruction

        Function<int()> g(f); // exercise copy() before destruction
        EXPECT_EQ(tracked.use_count(), 3);
        EXPECT_EQ(g(), 1);

        Function<int()> h(std::move(g));   // exercise move() before destruction
        EXPECT_EQ(tracked.use_count(), 3); // move transfers ownership, adds no reference
        EXPECT_EQ(h(), 1);
    } // f, g (moved-from), and h destroyed here
    EXPECT_EQ(tracked.use_count(), 1);
}

// Verifies destroying a Function holding a heap-stored callable
// releases that callable's own resources.
TEST(Destructor, DestructorReleasesHeap) {
    auto tracked = std::make_shared<int>(1);
    EXPECT_EQ(tracked.use_count(), 1);
    {
        Function<int()> f = LargePayload{tracked, {}};
        EXPECT_EQ(tracked.use_count(), 2);
    } // f destroyed here
    EXPECT_EQ(tracked.use_count(), 1);
}

// Verifies destroying an empty Function is safe.
TEST(Destructor, DestructorEmptySafe) {
    // clang-format off
    { Function<int()> f; }
    // clang-format on
    SUCCEED(); // reaching here without crashing is the test
}

// Verifies destroying a MoveOnlyFunction holding an SBO-stored callable
// releases that callable's own resources.
TEST(Destructor, MoveOnlyDestructorSbo) {
    auto tracked = std::make_shared<int>(1);
    EXPECT_EQ(tracked.use_count(), 1);
    {
        MoveOnlyFunction<int()> f = [tracked] { return *tracked; };
        EXPECT_EQ(tracked.use_count(), 2);
        EXPECT_EQ(f(), 1); // exercise invoke() before destruction

        MoveOnlyFunction<int()> g(std::move(f)); // exercise move() before destruction
        EXPECT_EQ(tracked.use_count(), 2);       // move transfers ownership, adds no reference
        EXPECT_EQ(g(), 1);
    } // f (moved-from) and g destroyed here
    EXPECT_EQ(tracked.use_count(), 1);
}

// Verifies destroying a MoveOnlyFunction holding a heap-stored callable
// releases that callable's own resources.
TEST(Destructor, MoveOnlyDestructorHeap) {
    auto tracked = std::make_shared<int>(1);
    EXPECT_EQ(tracked.use_count(), 1);
    {
        MoveOnlyFunction<int()> f = LargePayload{tracked, {}};
        EXPECT_EQ(tracked.use_count(), 2);
    } // f destroyed here
    EXPECT_EQ(tracked.use_count(), 1);
}

// Verifies destroying an empty MoveOnlyFunction is safe.
TEST(Destructor, MoveOnlyDestructorEmptySafe) {
    // clang-format off
    { MoveOnlyFunction<int()> f; }
    // clang-format on
    SUCCEED();
}

// Verifies destroying a FunctionRef does not affect the referenced
// external object -- FunctionRef never owned it, so its own resources
// are untouched by the FunctionRef going out of scope.
TEST(Destructor, RefDestructorNoEffect) {
    auto tracked = std::make_shared<int>(1);
    auto callable = [tracked] { return *tracked; };
    EXPECT_EQ(tracked.use_count(), 2); // one in tracked, one captured by callable
    {
        FunctionRef<int()> ref(callable);
        EXPECT_EQ(tracked.use_count(), 2); // FunctionRef adds no reference
    } // ref destroyed here
    EXPECT_EQ(tracked.use_count(), 2); // still just tracked + callable, unaffected
}
