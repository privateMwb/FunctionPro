#pragma once

#include <iostream>
#include <string>
#include <iomanip>

// ANSI terminal color codes.
constexpr const char* RESET  = "\033[0m";
constexpr const char* CYAN   = "\033[96m";
constexpr const char* GRAY   = "\033[37m";
constexpr const char* BLUE   = "\033[94m";

// Prints a horizontal separator line.
inline void borderLine() {
    std::cout << GRAY << std::string(70, '-') << RESET << "\n";
}

// Converts a snake_case function name to Title Case.
inline std::string prettify(std::string_view text)
{
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

// Prints the main title.
inline void mainTitle(std::string_view title) {
    std::cout << BLUE << prettify(title) << RESET << "\n";
}

// Prints a section title.
inline void setTitle(std::string_view title) {
    std::cout << CYAN << prettify(title) << RESET << "\n";
}