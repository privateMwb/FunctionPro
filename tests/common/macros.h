#include "helper.h"

// Executes a test function and prints a success message.
#define RUN(name) do {                          \
    try {                                       \
        int f = fail;                           \
        name();                                 \
        /* CHK failures inside name() increment `fail` without throwing, */ \
        /* so compare against the snapshot to detect them here. */ \
        if(f == fail){                          \
            std::cout << GREEN                  \
                      << "[PASS] "              \
                      << RESET                  \
                      << prettify(#name)        \
                      << "\n";                  \
        }                                       \
    } catch (const std::exception& e) {         \
        ++fail;                                 \
        std::cout << RED                        \
                  << "[FAIL] "                  \
                  << RESET                      \
                  << prettify(#name)            \
                  << " threw: "                 \
                  << e.what()                   \
                  << "\n";                      \
    } catch (...) {                             \
        ++fail;                                 \
        std::cout << RED                        \
                  << "[FAIL] "                  \
                  << RESET                      \
                  << prettify(#name)            \
                  << " threw unknown\n";        \
    }                                           \
} while (0)

// Evaluates a test condition and updates the test statistics.
#define CHK(expr) do {                  \
    if (!(expr)) {                      \
        ++fail;                         \
        std::cout << RED                \
                  << "[FAIL] "          \
                  << RESET              \
                  << "("                \
                  << __FILE__           \
                  << ":"                \
                  << __LINE__           \
                  << ")\n";             \
    } else ++pass;                      \
    ++total;                            \
} while (0)

// Verifies that evaluating `expr` throws an exception convertible to
// ExceptionType. Fails via CHK if no exception is thrown, or if a
// different exception type is thrown.
#define CHK_THROWS(expr, ExceptionType)     \
    do {                                    \
        bool threw = false;                \
        try {                               \
            (void)(expr);                  \
        } catch (const ExceptionType&) {    \
            threw = true;                   \
        } catch (...) {                     \
        }                                    \
        CHK(threw);                         \
    } while (0)
    
// Registers this file's run_tests with the global registry so it runs
// automatically at startup. The anonymous namespace gives `registrar`
// internal linkage, preventing duplicate-symbol errors when this macro
// is expanded in multiple translation units.
#define REGISTER_TEST_SUITE()        \
    namespace {                      \
        static TestRegistrar registrar( \
            __FILE__,                \
            run_tests                \
        );                           \
    }