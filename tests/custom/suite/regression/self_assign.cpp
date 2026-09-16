// FunctionPro self-assignment regression test suite.
//
// Not a historical bug -- confirmed safe during audit, kept here as a
// guard against a future change removing the explicit `this != &other`
// checks Function's copy-assign and move-assign both rely on. Without
// them, move-assign in particular would call reset() (destroying
// storage_), then immediately try to move *from* that same
// now-destroyed storage (since other.storage_ is storage_ when
// this == &other) -- undefined behavior. Stateful captures are used
// deliberately: a trivial captureless lambda has nothing to corrupt
// either way, so it wouldn't actually exercise the guard.
//
// Coverage:
// - Function: `f = f` (self-copy-assign) and `f = std::move(f)`
//   (self-move-assign) both leave a stateful f fully intact
// - MoveOnlyFunction: `f = std::move(f)` only -- copy-assign doesn't
//   exist to self-assign with
// - FunctionRef: `f = f` and `f = std::move(f)` -- trivial regardless,
//   but worth confirming it doesn't corrupt the pointer pair

#include <support/framework.h>

#include <string>

using namespace FunctionPro;

namespace {
struct StringCallable {
    std::string s;
    std::string operator()() const {
        return s;
    }
};
} // namespace

// Verifies self-copy-assign on a Function leaves stateful content intact.
static void function_self_copy_assign_is_safe() {
    Function<std::string()> f = StringCallable{"hello self-assign"};

    f = f; // NOLINT(clang-diagnostic-self-assign-overloaded)

    CHK(static_cast<bool>(f));
    CHK(f() == "hello self-assign");

    // Self-assign deliberately never calls copy() (that's the whole point
    // of the guard this file is testing), so exercise a genuine, non-self
    // copy here to cover that code path for this binding.
    Function<std::string()> g(f);
    CHK(g() == "hello self-assign");
}

// Verifies self-move-assign on a Function leaves stateful content intact.
static void function_self_move_assign_is_safe() {
    Function<std::string()> f = StringCallable{"hello self-assign"};

    f = std::move(f); // NOLINT(clang-diagnostic-self-move)

    CHK(static_cast<bool>(f));
    CHK(f() == "hello self-assign");
}

// Verifies self-move-assign on a MoveOnlyFunction leaves stateful
// content intact.
static void move_only_self_move_assign_is_safe() {
    MoveOnlyFunction<std::string()> f = StringCallable{"hello self-assign"};

    f = std::move(f); // NOLINT(clang-diagnostic-self-move)

    CHK(static_cast<bool>(f));
    CHK(f() == "hello self-assign");
}

// Verifies self-copy-assign on a FunctionRef leaves it fully intact.
static void function_ref_self_copy_assign_is_safe() {
    auto callable = [] { return 5; };
    FunctionRef<int()> f(callable);

    f = f; // NOLINT(clang-diagnostic-self-assign-overloaded)

    CHK(static_cast<bool>(f));
    CHK(f() == 5);
}

// Verifies self-move-assign on a FunctionRef leaves it fully intact.
static void function_ref_self_move_assign_is_safe() {
    auto callable = [] { return 5; };
    FunctionRef<int()> f(callable);

    f = std::move(f); // NOLINT(clang-diagnostic-self-move)

    CHK(static_cast<bool>(f));
    CHK(f() == 5);
}

// Executes all self-assignment regression test cases.
static void run_tests() {
    RUN(function_self_copy_assign_is_safe);
    RUN(function_self_move_assign_is_safe);

    RUN(move_only_self_move_assign_is_safe);

    RUN(function_ref_self_copy_assign_is_safe);
    RUN(function_ref_self_move_assign_is_safe);
}

REGISTER_TEST_SUITE();
