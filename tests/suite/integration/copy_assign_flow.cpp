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
static void copy_assign_replaces_existing_binding() {
    auto tracked = std::make_shared<int>(3);
    Function<int()> dst = [] { return -1; }; // starts bound to something else
    CHK(dst() == -1);                        // exercise invoke() for dst's initial binding

    {
        Function<int()> dstCopy(dst); // exercise copy() for dst's initial binding
        CHK(dstCopy() == -1);

        Function<int()> dstMoved(std::move(dstCopy)); // exercise move()
        CHK(dstMoved() == -1);
    } // dstCopy (moved-from) and dstMoved destroyed here, doesn't affect anything below

    Function<int()> src = [tracked] { return *tracked; };
    CHK(tracked.use_count() == 2);

    dst = src;

    CHK(tracked.use_count() == 3); // dst now holds its own captured copy
    CHK(dst() == 3);
    CHK(src() == 3);
}

// Verifies copy-assigning over a Function that previously held a
// heap-stored callable correctly releases the old heap resource before
// taking on the new binding.
static void copy_assign_releases_old_heap_resource() {
    auto oldTracked = std::make_shared<int>(1);
    auto newTracked = std::make_shared<int>(2);

    Function<int()> dst = LargePayload{oldTracked, {}};
    CHK(oldTracked.use_count() == 2);

    Function<int()> src = LargePayload{newTracked, {}};
    dst = src; // old heap-stored callable must be released here

    CHK(oldTracked.use_count() == 1); // released
    CHK(newTracked.use_count() == 3); // dst + src each hold a copy
    CHK(dst() == 2);
    CHK(src() == 2);
}

// Verifies copy-assigning a FunctionRef rebinds it to a different
// referenced object cleanly.
static void function_ref_copy_assign_rebinds_referent() {
    Multiplier a{2};
    Multiplier b{5};
    FunctionRef<int(int)> dst(a);
    FunctionRef<int(int)> src(b);

    CHK(dst(3) == 6);

    dst = src;

    CHK(dst(3) == 15); // now references b
    a.factor = 100;    // mutating the old referent no longer affects dst
    CHK(dst(3) == 15);
}

// Executes all copy-assign flow test cases.
static void run_tests() {
    RUN(copy_assign_replaces_existing_binding);
    RUN(copy_assign_releases_old_heap_resource);

    RUN(function_ref_copy_assign_rebinds_referent);
}

REGISTER_TEST_SUITE();
