// FunctionPro SBO self-pointer regression test suite.
//
// Not a historical bug -- confirmed safe during audit, kept here as a
// guard against a future change silently breaking the guarantee.
//
// std::string's short-string-optimized buffer typically contains an
// internal pointer to itself (its own inline buffer). A raw byte-swap
// of two SBO-stored objects like this would corrupt that internal
// pointer, since it would then point into the wrong object's memory.
// swap()'s doc comment explicitly calls this out as the reason it goes
// through each side's own vtable move instead of exchanging bytes
// directly. This file goes further than integration/swap_exchange.cpp:
// it verifies full string *content* survives correctly (not just
// length) across many repeated swap cycles, for both Function and
// MoveOnlyFunction.
//
// Coverage:
// - Repeated swap() cycles between two SBO-stored std::string-capturing
//   callables preserve full string content correctly, for Function and
//   MoveOnlyFunction

#include <gtest/gtest.h>

#include <string>

#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

namespace {
struct StringCallable {
    std::string s;
    std::string operator()() const {
        return s;
    }
};
} // namespace

// Verifies Function::swap() preserves full string content correctly
// across many repeated swap cycles.
TEST(SboSelfPointer, SwapPreservesString) {
    for (int i = 0; i < 10000; ++i) {
        std::string s1 = "left-" + std::to_string(i);
        std::string s2 = "right-" + std::to_string(i * 3);

        Function<std::string()> a = StringCallable{s1};
        Function<std::string()> b = StringCallable{s2};

        a.swap(b);
        ASSERT_EQ(a(), s2) << "at iteration " << i;
        ASSERT_EQ(b(), s1) << "at iteration " << i;

        a.swap(b); // swap back
        ASSERT_EQ(a(), s1) << "at iteration " << i;
        ASSERT_EQ(b(), s2) << "at iteration " << i;
    }
}

// Verifies MoveOnlyFunction::swap() preserves full string content
// correctly across many repeated swap cycles.
TEST(SboSelfPointer, MoveOnlySwapPreservesString) {
    for (int i = 0; i < 10000; ++i) {
        std::string s1 = "left-" + std::to_string(i);
        std::string s2 = "right-" + std::to_string(i * 3);

        MoveOnlyFunction<std::string()> a = StringCallable{s1};
        MoveOnlyFunction<std::string()> b = StringCallable{s2};

        a.swap(b);
        ASSERT_EQ(a(), s2) << "at iteration " << i;
        ASSERT_EQ(b(), s1) << "at iteration " << i;

        a.swap(b); // swap back
        ASSERT_EQ(a(), s1) << "at iteration " << i;
        ASSERT_EQ(b(), s2) << "at iteration " << i;
    }
}
