// FunctionPro SBO/heap boundary integration test suite.
//
// Coverage:
// - Reassigning the same Function/MoveOnlyFunction instance back and
//   forth across the SBO/heap boundary (small -> large -> small ->
//   large) works correctly at every transition, and each old binding's
//   resources are properly released before the new one takes over --
//   verified via a shared_ptr use_count that must return to baseline
//   after every reassignment
// - FunctionRef has no SBO/heap boundary of its own (it never copies
//   the referenced callable), so there's nothing to transition -- its
//   case here instead just confirms rebinding across differently-sized
//   external referents behaves correctly, included for completeness
//   rather than as a real boundary test

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

// Verifies a Function reassigned repeatedly across the SBO/heap
// boundary stays correct at every step, releasing each old binding's
// resources before taking on the new one.
static void function_transitions_across_boundary_repeatedly() {
    auto t1 = std::make_shared<int>(1);
    auto t2 = std::make_shared<int>(2);
    auto t3 = std::make_shared<int>(3);
    auto t4 = std::make_shared<int>(4);

    Function<int()> f = [t1] { return *t1; }; // small (SBO)
    CHK(f() == 1);
    CHK(t1.use_count() == 2);

    {
        Function<int()> copy1(f); // exercise copy() for this binding
        CHK(copy1() == 1);

        Function<int()> moved1(std::move(copy1)); // exercise move() for this binding
        CHK(moved1() == 1);
    } // copy1 (moved-from) and moved1 destroyed here, back to baseline before the next check

    f = LargePayload{t2, {}}; // large (heap)
    CHK(f() == 2);
    CHK(t1.use_count() == 1); // released
    CHK(t2.use_count() == 2);

    f = [t3] { return *t3; }; // back to small
    CHK(f() == 3);
    CHK(t2.use_count() == 1); // released
    CHK(t3.use_count() == 2);

    {
        Function<int()> copy3(f); // exercise copy() for this binding
        CHK(copy3() == 3);
    } // copy3 destroyed here, back to baseline before the next check

    f = LargePayload{t4, {}}; // large again
    CHK(f() == 4);
    CHK(t3.use_count() == 1); // released
    CHK(t4.use_count() == 2);
}

// Verifies a MoveOnlyFunction reassigned repeatedly across the SBO/heap
// boundary stays correct at every step, same as Function above.
static void move_only_transitions_across_boundary_repeatedly() {
    auto t1 = std::make_shared<int>(1);
    auto t2 = std::make_shared<int>(2);
    auto t3 = std::make_shared<int>(3);
    auto t4 = std::make_shared<int>(4);

    MoveOnlyFunction<int()> f = [t1] { return *t1; }; // small (SBO)
    CHK(f() == 1);
    CHK(t1.use_count() == 2);

    {
        MoveOnlyFunction<int()> moved1(std::move(f)); // exercise move() for this binding
        CHK(t1.use_count() == 2);                     // move transfers ownership, adds no reference
        CHK(moved1() == 1);
        f = std::move(moved1); // move back so the checks below are unaffected
    }

    f = LargePayload{t2, {}}; // large (heap)
    CHK(f() == 2);
    CHK(t1.use_count() == 1);
    CHK(t2.use_count() == 2);

    f = [t3] { return *t3; }; // back to small
    CHK(f() == 3);
    CHK(t2.use_count() == 1);
    CHK(t3.use_count() == 2);

    f = LargePayload{t4, {}}; // large again
    CHK(f() == 4);
    CHK(t3.use_count() == 1);
    CHK(t4.use_count() == 2);
}

// Verifies a FunctionRef reassigned between differently-sized external
// referents behaves correctly -- included for completeness, not
// because a boundary effect actually exists for it (see file header).
static void function_ref_rebinds_across_different_sized_referents() {
    int small = 5;
    auto smallCallable = [&small] { return small; };

    std::array<int, 20> bigData{};
    bigData[19] = 99;
    auto bigCallable = [&bigData] { return bigData[19]; };

    FunctionRef<int()> ref(smallCallable);
    CHK(ref() == 5);

    ref = bigCallable;
    CHK(ref() == 99);

    ref = smallCallable;
    CHK(ref() == 5);
}

// Executes all SBO/heap boundary test cases.
static void run_tests() {
    RUN(function_transitions_across_boundary_repeatedly);
    RUN(move_only_transitions_across_boundary_repeatedly);
    RUN(function_ref_rebinds_across_different_sized_referents);
}

REGISTER_TEST_SUITE();
