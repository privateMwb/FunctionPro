// FunctionPro FunctionRef dispatch test suite.
//
// Coverage:
// - FunctionRef bound to a raw function pointer (e.g. int(*)(int))
// - FunctionRef bound directly to a function type (decays to pointer)
// - FunctionRef bound to a callable object (lambda/functor)
// - Each dispatch path invokes correctly and forwards arguments
// - FunctionRef reflects live state of the referenced external
//   callable object -- it stores an address, not a copy

#include <support/framework.h>

using namespace FunctionPro;

namespace {
int addOne(int x) {
    return x + 1;
}

struct Multiplier {
    int factor;
    int operator()(int x) const {
        return x * factor;
    }
};
} // namespace

// Verifies FunctionRef bound to a raw function pointer invokes correctly.
static void dispatches_via_function_pointer() {
    int (*fp)(int) = &addOne;
    FunctionRef<int(int)> ref(fp);
    CHK(ref(4) == 5);
}

// Verifies FunctionRef bound directly to a function (not an explicit
// pointer variable) invokes correctly -- the function decays to a
// pointer at the call site.
static void dispatches_via_function_type() {
    FunctionRef<int(int)> ref(addOne);
    CHK(ref(9) == 10);
}

// Verifies FunctionRef bound to a callable object invokes correctly.
static void dispatches_via_callable_object() {
    Multiplier m{3};
    FunctionRef<int(int)> ref(m);
    CHK(ref(4) == 12);
}

// Verifies FunctionRef bound to a lambda invokes correctly and forwards
// arguments.
static void dispatches_via_lambda() {
    auto add = [](int a, int b) { return a + b; };
    FunctionRef<int(int, int)> ref(add);
    CHK(ref(2, 5) == 7);
}

// Verifies FunctionRef reflects live mutation of the referenced external
// callable object, since it stores an address rather than a copy.
static void reflects_live_state_of_referenced_object() {
    Multiplier m{2};
    FunctionRef<int(int)> ref(m);
    CHK(ref(5) == 10);

    m.factor = 10;     // mutate the external object after binding
    CHK(ref(5) == 50); // FunctionRef sees the change, not a stale copy
}

// Executes all FunctionRef dispatch test cases.
static void run_tests() {
    RUN(dispatches_via_function_pointer);
    RUN(dispatches_via_function_type);
    RUN(dispatches_via_callable_object);
    RUN(dispatches_via_lambda);
    RUN(reflects_live_state_of_referenced_object);
}

REGISTER_TEST_SUITE();
