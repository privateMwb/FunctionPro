// FunctionPro throwing copy constructor regression test suite.
//
// Not a historical bug -- confirmed safe during audit, kept here as a
// guard against a future change silently breaking the guarantee.
//
// Coverage:
// - Copy-assigning a Function whose callable throws mid-copy leaves the
//   destination completely unaffected (strong exception guarantee),
//   per the doc comment in Function.tpp: the source is copied into a
//   temporary first, and only swapped into `this` if that copy
//   succeeds
// - Copy-constructing a Function whose source callable throws mid-copy
//   does not crash, leak, or corrupt the source -- the new Function
//   object simply never comes into existence, same as any C++
//   constructor that throws

#include <support/framework.h>

using namespace FunctionPro;

namespace {
struct ThrowsOnNthCopy {
    int tag;
    static int copyCallCount;
    static int throwOnCall;

    explicit ThrowsOnNthCopy(int t) : tag(t) {}
    ThrowsOnNthCopy(const ThrowsOnNthCopy& other) : tag(other.tag) {
        ++copyCallCount;
        if (copyCallCount == throwOnCall) {
            throw std::runtime_error("simulated copy failure");
        }
    }
    int operator()() const {
        return tag;
    }
};
int ThrowsOnNthCopy::copyCallCount = 0;
int ThrowsOnNthCopy::throwOnCall = -1;
} // namespace

// Verifies a failed copy-assign leaves the destination completely
// unaffected.
static void copy_assign_throw_leaves_destination_unaffected() {
    Function<int()> dst = ThrowsOnNthCopy{111};
    Function<int()> src = ThrowsOnNthCopy{222};

    ThrowsOnNthCopy::copyCallCount = 0;
    ThrowsOnNthCopy::throwOnCall = 1;

    CHK_THROWS(dst = src, std::runtime_error);

    ThrowsOnNthCopy::throwOnCall = -1; // stop intercepting

    CHK(static_cast<bool>(dst));
    CHK(dst() == 111); // unaffected by the failed assignment
}

// Verifies a failed copy-construct does not crash, leak, or corrupt the
// source Function it copied from.
static void copy_construct_throw_leaves_source_unaffected() {
    Function<int()> src = ThrowsOnNthCopy{333};

    ThrowsOnNthCopy::copyCallCount = 0;
    ThrowsOnNthCopy::throwOnCall = 1;

    bool threw = false;
    try {
        Function<int()> dst(src); // should throw during the copy
        (void)dst;
    } catch (const std::runtime_error&) {
        threw = true;
    }

    ThrowsOnNthCopy::throwOnCall = -1;

    CHK(threw);
    CHK(static_cast<bool>(src)); // source untouched by the failed copy
    CHK(src() == 333);
}

// Executes all throwing-copy-constructor regression test cases.
static void run_tests() {
    RUN(copy_assign_throw_leaves_destination_unaffected);
    RUN(copy_construct_throw_leaves_source_unaffected);
}

REGISTER_TEST_SUITE();
