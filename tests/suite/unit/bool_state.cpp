// FunctionPro operator bool() test suite.
//
// Coverage:
// - Default-constructed instance reports false, for all three types
// - nullptr-constructed instance reports false (Function, MoveOnlyFunction)
// - Bound instance reports true, for all three types
// - After reset(), a previously-bound instance reports false
//   (Function, MoveOnlyFunction — FunctionRef has no reset())

#include <support/framework.h>

using namespace FunctionPro;

// Verifies a default-constructed Function reports false.
static void function_default_is_false() {
    Function<int()> f;
    CHK(!static_cast<bool>(f));
}

// Verifies a nullptr-constructed Function reports false.
static void function_nullptr_is_false() {
    Function<int()> f(nullptr);
    CHK(!static_cast<bool>(f));
}

// Verifies a bound Function reports true.
static void function_bound_is_true() {
    Function<int()> f = [] { return 1; };
    CHK(static_cast<bool>(f));
}

// Verifies a Function reports false after reset().
static void function_reset_is_false() {
    Function<int()> f = [] { return 1; };
    f.reset();
    CHK(!static_cast<bool>(f));
}

// Verifies a default-constructed MoveOnlyFunction reports false.
static void move_only_default_is_false() {
    MoveOnlyFunction<int()> f;
    CHK(!static_cast<bool>(f));
}

// Verifies a nullptr-constructed MoveOnlyFunction reports false.
static void move_only_nullptr_is_false() {
    MoveOnlyFunction<int()> f(nullptr);
    CHK(!static_cast<bool>(f));
}

// Verifies a bound MoveOnlyFunction reports true.
static void move_only_bound_is_true() {
    MoveOnlyFunction<int()> f = [] { return 1; };
    CHK(static_cast<bool>(f));
}

// Verifies a MoveOnlyFunction reports false after reset().
static void move_only_reset_is_false() {
    MoveOnlyFunction<int()> f = [] { return 1; };
    f.reset();
    CHK(!static_cast<bool>(f));
}

// Verifies a default-constructed FunctionRef reports false.
static void function_ref_default_is_false() {
    FunctionRef<int()> f;
    CHK(!static_cast<bool>(f));
}

// Verifies a bound FunctionRef reports true.
static void function_ref_bound_is_true() {
    auto callable = [] { return 1; };
    FunctionRef<int()> f(callable);
    CHK(static_cast<bool>(f));
}

// Executes all operator bool() test cases.
static void run_tests() {
    RUN(function_default_is_false);
    RUN(function_nullptr_is_false);
    RUN(function_bound_is_true);
    RUN(function_reset_is_false);

    RUN(move_only_default_is_false);
    RUN(move_only_nullptr_is_false);
    RUN(move_only_bound_is_true);
    RUN(move_only_reset_is_false);

    RUN(function_ref_default_is_false);
    RUN(function_ref_bound_is_true);
}

REGISTER_TEST_SUITE();
