// clang-format off
#include "helpers.h"      // prettify(), printTestLine()
                          // fail, pass, total
                          // GREEN, RED, DIM, RESET

#include <chrono>         // std::chrono::steady_clock
#include <sstream>        // std::ostringstream
// clang-format on

// Executes a test function and prints a check/cross result line with
// timing. Output written by the test itself (e.g. CHK failures) is
// captured and reprinted, indented, underneath that line — so the
// pass/fail mark always appears first, with any detail nested below it.
#define RUN(name)                                                                                  \
    do {                                                                                           \
        std::ostringstream __runBuf;                                                               \
        std::streambuf* __runOldBuf = std::cout.rdbuf(__runBuf.rdbuf());                           \
        auto __runStart = std::chrono::steady_clock::now();                                        \
        bool __runPassed = true;                                                                   \
        std::string __runExtra;                                                                    \
        try {                                                                                      \
            int f = fail;                                                                          \
            name();                                                                                \
            /* CHK failures inside name() increment `fail` without throwing, */                    \
            /* so compare against the snapshot to detect them here. */                             \
            if (f != fail)                                                                         \
                __runPassed = false;                                                               \
        } catch (const std::exception& e) {                                                        \
            ++fail;                                                                                \
            __runPassed = false;                                                                   \
            __runExtra = std::string("threw: ") + e.what();                                        \
        } catch (...) {                                                                            \
            ++fail;                                                                                \
            __runPassed = false;                                                                   \
            __runExtra = "threw unknown";                                                          \
        }                                                                                          \
        double __runElapsed = std::chrono::duration<double, std::milli>(                           \
                                  std::chrono::steady_clock::now() - __runStart)                   \
                                  .count();                                                        \
        std::cout.rdbuf(__runOldBuf);                                                              \
        printTestLine(__runPassed, prettify(#name), __runElapsed);                                 \
        std::cout << __runBuf.str();                                                               \
        if (!__runExtra.empty())                                                                   \
            std::cout << "      " << DIM << __runExtra << RESET << "\n";                           \
    } while (0)

// Evaluates a test condition and updates the test statistics. On
// failure, prints the failing expression and its location, indented
// so it nests visually under the enclosing RUN()'s result line.
#define CHK(expr)                                                                                  \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            ++fail;                                                                                \
            std::cout << "      " << RED << "\u2717 " << RESET << DIM << __FILE__ << ":"           \
                      << __LINE__ << RESET << "  " << #expr << "\n";                               \
        } else                                                                                     \
            ++pass;                                                                                \
        ++total;                                                                                   \
    } while (0)

// Verifies that evaluating `expr` throws an exception convertible to
// ExceptionType. Fails via CHK-style bookkeeping if no exception is
// thrown, or if a different exception type is thrown, printing an
// expected-vs-actual detail underneath.
#define CHK_THROWS(expr, ExceptionType)                                                            \
    do {                                                                                           \
        bool threw = false;                                                                        \
        bool wrongType = false;                                                                    \
        try {                                                                                      \
            (void)(expr);                                                                          \
        } catch (const ExceptionType&) {                                                           \
            threw = true;                                                                          \
        } catch (...) {                                                                            \
            wrongType = true;                                                                      \
        }                                                                                          \
        if (threw) {                                                                               \
            ++pass;                                                                                \
        } else {                                                                                   \
            ++fail;                                                                                \
            std::cout << "      " << RED << "\u2717 " << RESET << DIM << __FILE__ << ":"           \
                      << __LINE__ << RESET << "\n";                                                \
            std::cout << "          " << DIM << "expected: throw " << #ExceptionType << RESET      \
                      << "\n";                                                                     \
            std::cout << "          " << DIM                                                       \
                      << (wrongType ? "actual:   different exception type" : "actual:   no throw") \
                      << RESET << "\n";                                                            \
        }                                                                                          \
        ++total;                                                                                   \
    } while (0)

// Registers this file's run_tests with the global registry so it runs
// automatically at startup. The anonymous namespace gives `registrar`
// internal linkage, preventing duplicate-symbol errors when this macro
// is expanded in multiple translation units.
#define REGISTER_TEST_SUITE()                                                                      \
    namespace {                                                                                    \
    static TestRegistrar registrar(__FILE__, run_tests);                                           \
    }
