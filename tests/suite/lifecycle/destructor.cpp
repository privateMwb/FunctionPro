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

#include <support/framework.h>

#include <array>
#include <memory>

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
static void function_destructor_releases_sbo_resources() {
    auto tracked = std::make_shared<int>(1);
    CHK(tracked.use_count() == 1);
    {
        Function<int()> f = [tracked] { return *tracked; };
        CHK(tracked.use_count() == 2);
        CHK(f() == 1); // exercise invoke() before destruction

        Function<int()> g(f); // exercise copy() before destruction
        CHK(tracked.use_count() == 3);
        CHK(g() == 1);

        Function<int()> h(std::move(g)); // exercise move() before destruction
        CHK(tracked.use_count() == 3);   // move transfers ownership, adds no reference
        CHK(h() == 1);
    } // f, g (moved-from), and h destroyed here
    CHK(tracked.use_count() == 1);
}

// Verifies destroying a Function holding a heap-stored callable
// releases that callable's own resources.
static void function_destructor_releases_heap_resources() {
    auto tracked = std::make_shared<int>(1);
    CHK(tracked.use_count() == 1);
    {
        Function<int()> f = LargePayload{tracked, {}};
        CHK(tracked.use_count() == 2);
    } // f destroyed here
    CHK(tracked.use_count() == 1);
}

// Verifies destroying an empty Function is safe.
static void function_destructor_on_empty_is_safe() {
    // clang-format off
    { Function<int()> f; }
    // clang-format on
    CHK(true); // reaching here without crashing is the test
}

// Verifies destroying a MoveOnlyFunction holding an SBO-stored callable
// releases that callable's own resources.
static void move_only_destructor_releases_sbo_resources() {
    auto tracked = std::make_shared<int>(1);
    CHK(tracked.use_count() == 1);
    {
        MoveOnlyFunction<int()> f = [tracked] { return *tracked; };
        CHK(tracked.use_count() == 2);
        CHK(f() == 1); // exercise invoke() before destruction

        MoveOnlyFunction<int()> g(std::move(f)); // exercise move() before destruction
        CHK(tracked.use_count() == 2);           // move transfers ownership, adds no reference
        CHK(g() == 1);
    } // f (moved-from) and g destroyed here
    CHK(tracked.use_count() == 1);
}

// Verifies destroying a MoveOnlyFunction holding a heap-stored callable
// releases that callable's own resources.
static void move_only_destructor_releases_heap_resources() {
    auto tracked = std::make_shared<int>(1);
    CHK(tracked.use_count() == 1);
    {
        MoveOnlyFunction<int()> f = LargePayload{tracked, {}};
        CHK(tracked.use_count() == 2);
    } // f destroyed here
    CHK(tracked.use_count() == 1);
}

// Verifies destroying an empty MoveOnlyFunction is safe.
static void move_only_destructor_on_empty_is_safe() {
    // clang-format off
    { MoveOnlyFunction<int()> f; }
    // clang-format on
    CHK(true);
}

// Verifies destroying a FunctionRef does not affect the referenced
// external object -- FunctionRef never owned it, so its own resources
// are untouched by the FunctionRef going out of scope.
static void function_ref_destructor_does_not_affect_referenced_object() {
    auto tracked = std::make_shared<int>(1);
    auto callable = [tracked] { return *tracked; };
    CHK(tracked.use_count() == 2); // one in tracked, one captured by callable
    {
        FunctionRef<int()> ref(callable);
        CHK(tracked.use_count() == 2); // FunctionRef adds no reference
    } // ref destroyed here
    CHK(tracked.use_count() == 2); // still just tracked + callable, unaffected
}

// Executes all destructor test cases.
static void run_tests() {
    RUN(function_destructor_releases_sbo_resources);
    RUN(function_destructor_releases_heap_resources);
    RUN(function_destructor_on_empty_is_safe);

    RUN(move_only_destructor_releases_sbo_resources);
    RUN(move_only_destructor_releases_heap_resources);
    RUN(move_only_destructor_on_empty_is_safe);

    RUN(function_ref_destructor_does_not_affect_referenced_object);
}

REGISTER_TEST_SUITE();
