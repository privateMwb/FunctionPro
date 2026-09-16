// FunctionPro self-swap regression test suite.
//
// Not a historical bug -- confirmed safe during audit, kept here as a
// guard against a future change removing swap()'s explicit
// `if (this == &other) return;` early-out. Without that guard, the
// three-move rotation (this -> tmp -> other -> this) would use `this`'s
// storage as a move source *after* already having destroyed it in the
// first step, since `this == &other` means storage_ and other.storage_
// are the same memory -- undefined behavior. Stateful captures are used
// here deliberately: a trivial captureless lambda has nothing to
// corrupt either way, so it wouldn't actually exercise the guard.
//
// Coverage:
// - Function::swap(f) called on itself leaves f fully intact, for both
//   SBO-stored (stateful) and heap-stored callables
// - MoveOnlyFunction::swap(f) called on itself leaves f fully intact

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

struct StringCallable {
    std::string s;
    std::string operator()() const {
        return s;
    }
};
} // namespace

// Verifies self-swap on a Function holding a stateful SBO-stored
// callable is safe and leaves its content intact.
TEST(SelfSwap, FunctionSelfSwapSboIsSafe) {
    Function<std::string()> f = StringCallable{"hello self-swap"};

    f.swap(f); // NOLINT(clang-diagnostic-self-move)

    EXPECT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), "hello self-swap");
}

// Verifies self-swap on a Function holding a heap-stored callable is
// safe.
TEST(SelfSwap, FunctionSelfSwapHeapIsSafe) {
    Function<int()> f = LargePayload{{}, 42};

    f.swap(f); // NOLINT(clang-diagnostic-self-move)

    EXPECT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), 42);
}

// Verifies self-swap on a MoveOnlyFunction holding a stateful
// SBO-stored callable is safe and leaves its content intact.
TEST(SelfSwap, MoveOnlySelfSwapSboIsSafe) {
    MoveOnlyFunction<std::string()> f = StringCallable{"hello self-swap"};

    f.swap(f); // NOLINT(clang-diagnostic-self-move)

    EXPECT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), "hello self-swap");
}

// Verifies self-swap on a MoveOnlyFunction holding a heap-stored
// callable is safe.
TEST(SelfSwap, MoveOnlySelfSwapHeapIsSafe) {
    MoveOnlyFunction<int()> f = LargePayload{{}, 42};

    f.swap(f); // NOLINT(clang-diagnostic-self-move)

    EXPECT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), 42);
}
