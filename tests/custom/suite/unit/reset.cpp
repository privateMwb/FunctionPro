// FunctionPro reset() test suite.
//
// Coverage:
// - After reset(), the instance reports empty (operator bool() is
//   false) and invoking it throws std::bad_function_call
// - reset() on an already-empty instance is a safe no-op
// - reset() actually destroys the stored callable's resources (verified
//   via a shared_ptr use_count, not just that the API reports empty)
// - FunctionRef has no reset() and is not covered here

#include <support/framework.h>

#include <memory>

using namespace FunctionPro;

// Verifies Function::reset() leaves the instance empty and unusable.
static void function_reset_leaves_empty() {
    Function<int()> f = [] { return 1; };
    CHK(f() == 1); // exercise invoke() while still bound

    Function<int()> g(f); // exercise copy() while still bound
    CHK(g() == 1);

    Function<int()> h(std::move(g)); // exercise move() while still bound
    CHK(h() == 1);

    f.reset();

    CHK(!static_cast<bool>(f));
    CHK_THROWS(f(), std::bad_function_call);
}

// Verifies Function::reset() on an already-empty instance is a safe no-op.
static void function_reset_on_empty_is_noop() {
    Function<int()> f;
    f.reset();
    CHK(!static_cast<bool>(f));

    f.reset(); // second call, still nothing to release
    CHK(!static_cast<bool>(f));
}

// Verifies Function::reset() actually releases the stored callable's
// resources, not just that the API reports empty afterward.
static void function_reset_releases_resources() {
    auto tracked = std::make_shared<int>(1);
    CHK(tracked.use_count() == 1);

    Function<int()> f = [tracked] { return *tracked; };
    CHK(tracked.use_count() == 2); // captured copy inside f
    CHK(f() == 1);                 // exercise invoke() while still bound

    {
        Function<int()> g(f); // exercise copy() while still bound
        CHK(tracked.use_count() == 3);
        CHK(g() == 1);

        Function<int()> h(std::move(g)); // exercise move() while still bound
        CHK(tracked.use_count() == 3);   // move transfers ownership, adds no reference
        CHK(h() == 1);
    } // g (moved-from) and h destroyed here, back to baseline before reset() below

    f.reset();
    CHK(tracked.use_count() == 1); // f's copy was destroyed
}

// Verifies MoveOnlyFunction::reset() leaves the instance empty and
// unusable.
static void move_only_reset_leaves_empty() {
    MoveOnlyFunction<int()> f = [] { return 1; };
    CHK(f() == 1); // exercise invoke() while still bound

    MoveOnlyFunction<int()> g(std::move(f)); // exercise move() while still bound
    CHK(g() == 1);
    f = std::move(g); // move back so f.reset() below still resets a bound instance

    f.reset();

    CHK(!static_cast<bool>(f));
    CHK_THROWS(f(), std::bad_function_call);
}

// Verifies MoveOnlyFunction::reset() on an already-empty instance is a
// safe no-op.
static void move_only_reset_on_empty_is_noop() {
    MoveOnlyFunction<int()> f;
    f.reset();
    CHK(!static_cast<bool>(f));

    f.reset();
    CHK(!static_cast<bool>(f));
}

// Verifies MoveOnlyFunction::reset() actually releases the stored
// callable's resources.
static void move_only_reset_releases_resources() {
    auto tracked = std::make_shared<int>(1);
    CHK(tracked.use_count() == 1);

    MoveOnlyFunction<int()> f = [tracked] { return *tracked; };
    CHK(tracked.use_count() == 2);
    CHK(f() == 1); // exercise invoke() while still bound

    {
        MoveOnlyFunction<int()> g(std::move(f)); // exercise move() while still bound
        CHK(tracked.use_count() == 2);           // move transfers ownership, adds no reference
        CHK(g() == 1);
        f = std::move(g); // move back so the checks below are unaffected
    }

    f.reset();
    CHK(tracked.use_count() == 1);
}

// Executes all reset() test cases.
static void run_tests() {
    RUN(function_reset_leaves_empty);
    RUN(function_reset_on_empty_is_noop);
    RUN(function_reset_releases_resources);

    RUN(move_only_reset_leaves_empty);
    RUN(move_only_reset_on_empty_is_noop);
    RUN(move_only_reset_releases_resources);
}

REGISTER_TEST_SUITE();
