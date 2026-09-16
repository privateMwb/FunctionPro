#pragma once

// clang-format off
#include <algorithm>     // std::transform, std::max
#include <cctype>        // std::toupper, std::tolower
#include <initializer_list> // std::initializer_list (drawBorder)
#include <iostream>      // std::cout
#include <string>        // std::string
#include <string_view>   // std::string_view
// clang-format on

// ── Constants ───────────────────────────────────────────────────────

// ANSI terminal color codes.
// clang-format off
inline constexpr const char* RESET  = "\033[0m";
inline constexpr const char* BOLD   = "\033[1m";
inline constexpr const char* DIM    = "\033[2m";
inline constexpr const char* GREEN  = "\033[92m";
inline constexpr const char* RED    = "\033[91m";
inline constexpr const char* YELLOW = "\033[93m";
inline constexpr const char* CYAN   = "\033[96m";
inline constexpr const char* GRAY   = "\033[37m";
inline constexpr const char* BLUE   = "\033[94m";
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

// Prints the top-level title for the whole example run.
inline void mainTitle(std::string_view title) {
    std::cout << BLUE << prettify(title) << RESET << "\n";
}

// Prints a section title within an example -- used directly by suite
// files to label sub-parts of a single example's output (e.g. several
// demonstrated variations in one suite), not just by the framework's
// own drivers.
inline void setTitle(std::string_view title) {
    std::cout << CYAN << prettify(title) << RESET << "\n";
}

// ── Box drawing ──────────────────────────────────────────────────────
//
// Same glyphs, widths-by-content approach, and helper shapes as the
// test/benchmark tools' own box-drawing code, kept minimal here since
// this tool only needs a single small stat box (no comparison tables).

// clang-format off
inline constexpr const char* H  = "─";
inline constexpr const char* V  = "│";
inline constexpr const char* TL = "┌"; inline constexpr const char* TM = "┬"; inline constexpr const char* TR = "┐";
inline constexpr const char* ML = "├"; inline constexpr const char* MM = "┼"; inline constexpr const char* MR = "┤";
inline constexpr const char* BL = "└"; inline constexpr const char* BM = "┴"; inline constexpr const char* BR = "┘";
// clang-format on

// Repeats a (possibly multi-byte) UTF-8 token `n` times.
inline std::string repeat(const char* token, int n) {
    std::string out;
    out.reserve(static_cast<std::size_t>(n) * 3);
    for (int i = 0; i < n; ++i)
        out += token;
    return out;
}

// Counts display columns, not bytes -- multi-byte UTF-8 (the tree's
// "├─"/"└─" connectors) would otherwise overcount and drift a box's
// padding out from under its border.
inline int visualWidth(const std::string& s) {
    int width = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80)
            ++width;
    return width;
}

// Left-pads a cell's content out to `width` display columns.
inline std::string padCell(const std::string& s, int width) {
    const int w = visualWidth(s);
    return w >= width ? s : s + std::string(static_cast<std::size_t>(width - w), ' ');
}

// Draws one border row (top/mid/bottom) across the given column widths.
inline void drawBorder(const char* left, const char* mid, const char* right,
                       std::initializer_list<int> widths) {
    std::cout << GRAY << left;
    bool first = true;
    for (int w : widths) {
        if (!first)
            std::cout << mid;
        first = false;
        std::cout << repeat(H, w);
    }
    std::cout << right << RESET << "\n";
}