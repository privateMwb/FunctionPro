#pragma once

#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>
#include <FunctionPro/FunctionRef.h>

#include <iostream>
#include <string>

// Global test counters.
inline int total  = 0;
inline int pass   = 0;
inline int fail   = 0;

// ANSI terminal color codes.
constexpr const char* RESET = "\033[0m";
constexpr const char* GREEN = "\033[92m";
constexpr const char* RED   = "\033[91m";
constexpr const char* CYAN  = "\033[96m";
constexpr const char* GRAY  = "\033[37m";

// Prints a horizontal separator line.
inline void borderLine() {
    std::cout << GRAY << std::string(70, '-') << RESET << "\n";
}

// Prints the overall test statistics.
inline void stats() {
    std::cout << "T: " << total << "\n";
    std::cout << "P: " << pass << "\n";
    std::cout << "F: " << fail << "\n";
}

// Prints a test suite title.
inline void setTitle(std::string_view title) {
    std::cout << CYAN << title << RESET << "\n";
}

// Converts a snake_case function name to Title Case.
inline std::string prettify(const char* name) {
    std::string result{name};
    bool firstLetter = true;

    for (char& c : result) {
        if (firstLetter) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            firstLetter = false;
        } else {
            if (c == '_') {
                c = ' ';
                firstLetter = true;
            }
        }
    }
    return result;
}

// Executes a test function and prints a success message.
#define RUN(name) do {                          \
    try {                                       \
        int f = fail;                           \
        name();                                 \
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