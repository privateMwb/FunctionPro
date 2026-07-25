// FunctionPro swap exchange integration test suite.
//
// Coverage:
// - swap() correctly exchanges contents across all three storage-kind
//   combinations: SBO<->SBO, heap<->heap, and the mixed SBO<->heap
//   case, for both Function and MoveOnlyFunction. Each is tested
//   separately rather than assuming Function's result carries over --
//   MoveOnlyFunction is a distinct template instantiation with its own
//   getMoveOnly() vtable path, and could have a bug Function doesn't.
// - The mixed case in particular exercises the documented risk swap()
//   is designed around: swapping goes through each side's own vtable
//   move rather than a raw byte-swap, specifically to avoid corrupting
//   self-referential SBO-stored objects (e.g. a captured std::string
//   using short-string optimization) -- verified here with exactly
//   that kind of capture.
// - FunctionRef is excluded: it has no swap() member at all.

#include <support/framework.h>

#include <array>
#include <string>

using namespace FunctionPro;

namespace {
// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit.
struct LargePayload {
    std::array<std::byte, 64> padding{};
    int tag;
    int operator()() const {
        return tag;
    }
};

// Captures a short std::string -- small enough to trigger SSO on
// libstdc++/libc++, and small enough to also fit FunctionPro's own SBO.
// This is the exact self-referential-pointer scenario swap()'s design
// is meant to survive.
struct StringCallable {
    std::string s;
    int operator()() const {
        return static_cast<int>(s.size());
    }
};
} // namespace

// Verifies Function::swap() between two SBO-stored callables.
static void function_swap_sbo_sbo() {
    Function<int()> a = StringCallable{std::string(4, 'a')};
    Function<int()> b = StringCallable{std::string(9, 'b')};

    a.swap(b);

    CHK(a() == 9);
    CHK(b() == 4);

    // swap() never calls copy() (it moves), so exercise a genuine copy
    // here to cover that code path for this binding.
    Function<int()> c(a);
    CHK(c() == 9);
}

// Verifies Function::swap() between two heap-stored callables.
static void function_swap_heap_heap() {
    Function<int()> a = LargePayload{{}, 1};
    Function<int()> b = LargePayload{{}, 2};

    a.swap(b);

    CHK(a() == 2);
    CHK(b() == 1);
}

// Verifies Function::swap() between an SBO-stored callable and a
// heap-stored one, the mixed case.
static void function_swap_sbo_heap_mixed() {
    Function<int()> a = StringCallable{std::string(6, 'x')};
    Function<int()> b = LargePayload{{}, 77};

    a.swap(b);

    CHK(a() == 77);
    CHK(b() == 6);
}

// Verifies MoveOnlyFunction::swap() between two SBO-stored callables.
static void move_only_swap_sbo_sbo() {
    MoveOnlyFunction<int()> a = StringCallable{std::string(4, 'a')};
    MoveOnlyFunction<int()> b = StringCallable{std::string(9, 'b')};

    a.swap(b);

    CHK(a() == 9);
    CHK(b() == 4);
}

// Verifies MoveOnlyFunction::swap() between two heap-stored callables.
static void move_only_swap_heap_heap() {
    MoveOnlyFunction<int()> a = LargePayload{{}, 1};
    MoveOnlyFunction<int()> b = LargePayload{{}, 2};

    a.swap(b);

    CHK(a() == 2);
    CHK(b() == 1);
}

// Verifies MoveOnlyFunction::swap() between an SBO-stored callable and
// a heap-stored one, the mixed case.
static void move_only_swap_sbo_heap_mixed() {
    MoveOnlyFunction<int()> a = StringCallable{std::string(6, 'x')};
    MoveOnlyFunction<int()> b = LargePayload{{}, 77};

    a.swap(b);

    CHK(a() == 77);
    CHK(b() == 6);
}

// Executes all swap exchange test cases.
static void run_tests() {
    RUN(function_swap_sbo_sbo);
    RUN(function_swap_heap_heap);
    RUN(function_swap_sbo_heap_mixed);

    RUN(move_only_swap_sbo_sbo);
    RUN(move_only_swap_heap_heap);
    RUN(move_only_swap_sbo_heap_mixed);
}

REGISTER_TEST_SUITE();
