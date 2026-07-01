#pragma once

#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>
#include <FunctionPro/FunctionRef.h>

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

// Prints the main title.
inline void mainTitle(std::string_view title) {
    std::cout << BLUE << title << RESET << "\n";
}

// Prints a section title.
inline void setTitle(std::string_view title) {
    std::cout << CYAN << title << RESET << "\n";
}