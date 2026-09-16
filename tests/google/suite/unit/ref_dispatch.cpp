// FunctionPro FunctionRef dispatch test suite.
//
// Coverage:
// - FunctionRef bound to a raw function pointer (e.g. int(*)(int))
// - FunctionRef bound directly to a function type (decays to pointer)
// - FunctionRef bound to a callable object (lambda/functor)
// - Each dispatch path invokes correctly and forwards arguments
// - FunctionRef reflects live state of the referenced external
//   callable object -- it stores an address, not a copy

#include <gtest/gtest.h>

#include <FunctionPro/FunctionRef.h>

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
TEST(RefDispatch, DispatchesViaFunctionPointer) {
    int (*fp)(int) = &addOne;
    FunctionRef<int(int)> ref(fp);
    EXPECT_EQ(ref(4), 5);
}

// Verifies FunctionRef bound directly to a function (not an explicit
// pointer variable) invokes correctly -- the function decays to a
// pointer at the call site.
TEST(RefDispatch, DispatchesViaFunctionType) {
    FunctionRef<int(int)> ref(addOne);
    EXPECT_EQ(ref(9), 10);
}

// Verifies FunctionRef bound to a callable object invokes correctly.
TEST(RefDispatch, DispatchesViaCallableObject) {
    Multiplier m{3};
    FunctionRef<int(int)> ref(m);
    EXPECT_EQ(ref(4), 12);
}

// Verifies FunctionRef bound to a lambda invokes correctly and forwards
// arguments.
TEST(RefDispatch, DispatchesViaLambda) {
    auto add = [](int a, int b) { return a + b; };
    FunctionRef<int(int, int)> ref(add);
    EXPECT_EQ(ref(2, 5), 7);
}

// Verifies FunctionRef reflects live mutation of the referenced external
// callable object, since it stores an address rather than a copy.
TEST(RefDispatch, ReflectsLiveStateOfReferencedObject) {
    Multiplier m{2};
    FunctionRef<int(int)> ref(m);
    EXPECT_EQ(ref(5), 10);

    m.factor = 10;         // mutate the external object after binding
    EXPECT_EQ(ref(5), 50); // FunctionRef sees the change, not a stale copy
}
