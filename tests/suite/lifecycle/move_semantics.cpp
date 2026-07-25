// FunctionPro move constructor test suite.
//
// Coverage:
// - Function/MoveOnlyFunction move construct, for both SBO-stored and
//   heap-stored callables: destination invokes correctly, source is
//   left empty (operator bool() is false) -- both types explicitly null
//   the source's vtable pointer as part of their custom move
// - FunctionRef move construct is the contrasting case: its move
//   constructor is defaulted (a trivial memberwise copy of the pointer
//   pair), so the source is NOT left empty -- it still references the
//   same external object after being "moved from". This is real,
//   observable behavior worth documenting explicitly rather than
//   assuming it matches the other two types' contract.

#include <support/framework.h>

#include <array>

using namespace FunctionPro;

namespace {
constexpr int kPad = 3; // keeps the small lambda's capture well under 40 bytes

// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit.
struct LargePayload {
    std::array<std::byte, 64> padding{};
    int tag;
    int operator()() const {
        return tag;
    }
};
} // namespace

// Verifies move-constructing a Function from an SBO-stored callable
// transfers correctly and leaves the source empty.
static void function_move_sbo_transfers_and_empties_source() {
    int a = kPad, b = kPad, c = kPad;
    Function<int()> src = [a, b, c] { return a + b + c; };

    Function<int()> dst(std::move(src));

    CHK(dst() == 9);
    CHK(!static_cast<bool>(src));

    Function<int()> other(dst); // exercise copy() for this binding
    CHK(other() == 9);
}

// Verifies move-constructing a Function from a heap-stored callable
// transfers correctly and leaves the source empty.
static void function_move_heap_transfers_and_empties_source() {
    Function<int()> src = LargePayload{{}, 42};

    Function<int()> dst(std::move(src));

    CHK(dst() == 42);
    CHK(!static_cast<bool>(src));
}

// Verifies move-constructing a MoveOnlyFunction from an SBO-stored
// callable transfers correctly and leaves the source empty.
static void move_only_move_sbo_transfers_and_empties_source() {
    int a = kPad, b = kPad, c = kPad;
    MoveOnlyFunction<int()> src = [a, b, c] { return a + b + c; };

    MoveOnlyFunction<int()> dst(std::move(src));

    CHK(dst() == 9);
    CHK(!static_cast<bool>(src));
}

// Verifies move-constructing a MoveOnlyFunction from a heap-stored
// callable transfers correctly and leaves the source empty.
static void move_only_move_heap_transfers_and_empties_source() {
    MoveOnlyFunction<int()> src = LargePayload{{}, 42};

    MoveOnlyFunction<int()> dst(std::move(src));

    CHK(dst() == 42);
    CHK(!static_cast<bool>(src));
}

// Verifies move-constructing a FunctionRef transfers correctly to the
// destination -- and, unlike the other two types, does NOT leave the
// source empty, since FunctionRef's move constructor is a defaulted
// trivial copy rather than a custom nulling move.
static void function_ref_move_does_not_empty_source() {
    auto callable = [] { return 7; };
    FunctionRef<int()> src(callable);

    FunctionRef<int()> dst(std::move(src));

    CHK(dst() == 7);
    CHK(static_cast<bool>(src)); // still true -- src was not nulled
    CHK(src() == 7);             // still fully usable, references the same object
}

// Executes all move constructor test cases.
static void run_tests() {
    RUN(function_move_sbo_transfers_and_empties_source);
    RUN(function_move_heap_transfers_and_empties_source);

    RUN(move_only_move_sbo_transfers_and_empties_source);
    RUN(move_only_move_heap_transfers_and_empties_source);

    RUN(function_ref_move_does_not_empty_source);
}

REGISTER_TEST_SUITE();
