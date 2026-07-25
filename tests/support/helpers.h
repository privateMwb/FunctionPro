#pragma once

// clang-format off
#include <algorithm>     // std::transform
#include <cctype>        // std::toupper, std::tolower
#include <iostream>      // std::cout
#include <string>        // std::string
#include <string_view>   // std::string_view
// clang-format on

// ── Constants ───────────────────────────────────────────────────────

// Global test counters.
// clang-format off
inline int total  = 0;
inline int pass   = 0;
inline int fail   = 0;

// ANSI terminal color codes.
inline constexpr const char* RESET = "\033[0m";
inline constexpr const char* GREEN = "\033[92m";
inline constexpr const char* RED   = "\033[91m";
inline constexpr const char* CYAN  = "\033[96m";
inline constexpr const char* GRAY  = "\033[37m";
// clang-format on

// ── String utilities ────────────────────────────────────────────────

// Converts a snake_case function name to Title Case.
inline std::string prettify(std::string_view text) {
    std::string result{text};
    bool firstLetter = true;

    for (char& c : result) {
        if (firstLetter) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            firstLetter = false;
        } else if (c == '_') {
            c = ' ';
            firstLetter = true;
        }
    }

    return result;
}

// Converts a string to lowercase.
inline std::string toLower(std::string_view str) {
    std::string result(str);

    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return result;
}

// ── Output / printing ───────────────────────────────────────────────

// Prints a horizontal separator line.
inline void borderLine() {
    std::cout << GRAY << std::string(70, '-') << RESET << "\n";
}

// Prints a test suite title.
inline void setTitle(std::string_view title) {
    std::cout << CYAN << prettify(title) << RESET << "\n";
}

// Prints the overall test statistics: total run, passed, and failed.
inline void stats() {
    std::cout << "T: " << total << "\n";
    std::cout << "P: " << pass << "\n";
    std::cout << "F: " << fail << "\n";
}