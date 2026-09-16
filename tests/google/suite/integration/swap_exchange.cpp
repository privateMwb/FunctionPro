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

#include <gtest/gtest.h>

#include <array>
#include <string>

#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>

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
TEST(SwapExchange, FunctionSwapSboSbo) {
    Function<int()> a = StringCallable{std::string(4, 'a')};
    Function<int()> b = StringCallable{std::string(9, 'b')};

    a.swap(b);

    EXPECT_EQ(a(), 9);
    EXPECT_EQ(b(), 4);

    // swap() never calls copy() (it moves), so exercise a genuine copy
    // here to cover that code path for this binding.
    Function<int()> c(a);
    EXPECT_EQ(c(), 9);
}

// Verifies Function::swap() between two heap-stored callables.
TEST(SwapExchange, FunctionSwapHeapHeap) {
    Function<int()> a = LargePayload{{}, 1};
    Function<int()> b = LargePayload{{}, 2};

    a.swap(b);

    EXPECT_EQ(a(), 2);
    EXPECT_EQ(b(), 1);
}

// Verifies Function::swap() between an SBO-stored callable and a
// heap-stored one, the mixed case.
TEST(SwapExchange, FunctionSwapSboHeapMixed) {
    Function<int()> a = StringCallable{std::string(6, 'x')};
    Function<int()> b = LargePayload{{}, 77};

    a.swap(b);

    EXPECT_EQ(a(), 77);
    EXPECT_EQ(b(), 6);
}

// Verifies MoveOnlyFunction::swap() between two SBO-stored callables.
TEST(SwapExchange, MoveOnlySwapSboSbo) {
    MoveOnlyFunction<int()> a = StringCallable{std::string(4, 'a')};
    MoveOnlyFunction<int()> b = StringCallable{std::string(9, 'b')};

    a.swap(b);

    EXPECT_EQ(a(), 9);
    EXPECT_EQ(b(), 4);
}

// Verifies MoveOnlyFunction::swap() between two heap-stored callables.
TEST(SwapExchange, MoveOnlySwapHeapHeap) {
    MoveOnlyFunction<int()> a = LargePayload{{}, 1};
    MoveOnlyFunction<int()> b = LargePayload{{}, 2};

    a.swap(b);

    EXPECT_EQ(a(), 2);
    EXPECT_EQ(b(), 1);
}

// Verifies MoveOnlyFunction::swap() between an SBO-stored callable and
// a heap-stored one, the mixed case.
TEST(SwapExchange, MoveOnlySwapSboHeapMixed) {
    MoveOnlyFunction<int()> a = StringCallable{std::string(6, 'x')};
    MoveOnlyFunction<int()> b = LargePayload{{}, 77};

    a.swap(b);

    EXPECT_EQ(a(), 77);
    EXPECT_EQ(b(), 6);
}
