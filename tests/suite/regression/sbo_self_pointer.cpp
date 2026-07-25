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

#include <support/framework.h>

#include <string>

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
static void function_swap_preserves_string_content_repeatedly() {
    bool allCorrect = true;
    for (int i = 0; i < 10000 && allCorrect; ++i) {
        std::string s1 = "left-" + std::to_string(i);
        std::string s2 = "right-" + std::to_string(i * 3);

        Function<std::string()> a = StringCallable{s1};
        Function<std::string()> b = StringCallable{s2};

        a.swap(b);
        if (a() != s2 || b() != s1) {
            allCorrect = false;
            break;
        }

        a.swap(b); // swap back
        if (a() != s1 || b() != s2) {
            allCorrect = false;
        }
    }
    CHK(allCorrect);
}

// Verifies MoveOnlyFunction::swap() preserves full string content
// correctly across many repeated swap cycles.
static void move_only_swap_preserves_string_content_repeatedly() {
    bool allCorrect = true;
    for (int i = 0; i < 10000 && allCorrect; ++i) {
        std::string s1 = "left-" + std::to_string(i);
        std::string s2 = "right-" + std::to_string(i * 3);

        MoveOnlyFunction<std::string()> a = StringCallable{s1};
        MoveOnlyFunction<std::string()> b = StringCallable{s2};

        a.swap(b);
        if (a() != s2 || b() != s1) {
            allCorrect = false;
            break;
        }

        a.swap(b); // swap back
        if (a() != s1 || b() != s2) {
            allCorrect = false;
        }
    }
    CHK(allCorrect);
}

// Executes all SBO self-pointer regression test cases.
static void run_tests() {
    RUN(function_swap_preserves_string_content_repeatedly);
    RUN(move_only_swap_preserves_string_content_repeatedly);
}

REGISTER_TEST_SUITE();
