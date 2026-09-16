// FunctionPro non-copyable-callable test suite.
//
// Coverage:
// - MoveOnlyFunction can wrap a callable that is not copy-constructible
//   (e.g. one capturing a std::unique_ptr) — something Function cannot
//   do, since Function's constructor requires copy-constructibility.
// - The wrapped callable is invoked directly, without any internal copy
//   being made along the way — verified by capturing state that would
//   only be reachable through the original object, not a copy.
// - Moving a MoveOnlyFunction wrapping such a callable transfers it
//   correctly (no attempt to copy the non-copyable payload).
// - FunctionRef can also reference a non-copy-constructible callable,
//   for a completely different reason than MoveOnlyFunction: it never
//   copies anything in the first place, so the referenced object's
//   copy-constructibility is irrelevant to it. Worth showing as a
//   contrast — MoveOnlyFunction handles this by owning a moved-in
//   object, FunctionRef by not owning anything at all.

#include <support/framework.h>

#include <memory>

using namespace FunctionPro;

// Verifies MoveOnlyFunction can bind and invoke a callable capturing a
// move-only std::unique_ptr.
static void wraps_unique_ptr_capturing_callable() {
    auto owned = std::make_unique<int>(77);

    MoveOnlyFunction<int()> f = [p = std::move(owned)] { return *p; };

    CHK(f() == 77);

    MoveOnlyFunction<int()> g(std::move(f)); // exercise move() for this binding
    CHK(g() == 77);
}

// Verifies the wrapped non-copyable callable is invoked directly (its
// captured state is read through the single live instance, not a copy
// -- if the library tried to copy it internally, this wouldn't compile
// or wouldn't reflect state correctly).
static void invocation_reflects_live_captured_state() {
    auto counter = std::make_unique<int>(0);

    MoveOnlyFunction<int()> f = [p = std::move(counter)]() mutable {
        ++(*p);
        return *p;
    };

    CHK(f() == 1);
    CHK(f() == 2);
    CHK(f() == 3);

    MoveOnlyFunction<int()> g(std::move(f)); // exercise move() for this binding
    CHK(g() == 4);                           // state (the counter) carries over through the move
}

// Verifies moving a MoveOnlyFunction wrapping a non-copyable callable
// transfers it correctly to the destination.
static void move_transfers_non_copyable_payload() {
    auto owned = std::make_unique<int>(99);
    MoveOnlyFunction<int()> src = [p = std::move(owned)] { return *p; };

    MoveOnlyFunction<int()> dst(std::move(src));

    CHK(dst() == 99);
    CHK(!static_cast<bool>(src));
}

// Verifies FunctionRef can reference a non-copy-constructible callable
// too -- but for a different reason than MoveOnlyFunction: it never
// copies the callable at all, so copy-constructibility never enters
// into it. The callable lives on the caller's stack; FunctionRef only
// stores its address.
static void ref_can_reference_non_copyable_callable() {
    auto owned = std::make_unique<int>(55);
    auto callable = [p = std::move(owned)] { return *p; };

    FunctionRef<int()> ref(callable);

    CHK(ref() == 55);
}

// Executes all non-copyable-callable test cases.
static void run_tests() {
    RUN(wraps_unique_ptr_capturing_callable);
    RUN(invocation_reflects_live_captured_state);
    RUN(move_transfers_non_copyable_payload);
    RUN(ref_can_reference_non_copyable_callable);
}

REGISTER_TEST_SUITE();
