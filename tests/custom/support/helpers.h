#pragma once

// clang-format off
#include <algorithm>     // std::transform
#include <cctype>        // std::toupper, std::tolower
#include <iomanip>       // std::setw, std::setprecision
#include <iostream>      // std::cout
#include <sstream>       // std::ostringstream
#include <string>        // std::string
#include <string_view>   // std::string_view
#include <vector>        // std::vector
// clang-format on

// ── Constants ───────────────────────────────────────────────────────

// Global test counters.
// clang-format off
inline int total  = 0;
inline int pass   = 0;
inline int fail   = 0;

// ANSI terminal color codes.
inline constexpr const char* RESET  = "\033[0m";
inline constexpr const char* BOLD   = "\033[1m";
inline constexpr const char* DIM    = "\033[2m";
inline constexpr const char* GREEN  = "\033[92m";
inline constexpr const char* RED    = "\033[91m";
inline constexpr const char* YELLOW = "\033[93m";
inline constexpr const char* CYAN   = "\033[96m";
inline constexpr const char* GRAY   = "\033[37m";
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
    std::cout << BOLD << CYAN << prettify(title) << RESET << "\n";
}

// Prints one RUN() result line: a colored check/cross mark, the
// (prettified) test name, and how long it took.
inline void printTestLine(bool passed, std::string_view name, double elapsedMs) {
    const char* mark = passed ? "\u2713" : "\u2717";
    const char* color = passed ? GREEN : RED;

    std::cout << "  " << color << mark << " " << RESET << std::left << std::setw(42) << name << DIM
              << std::right << std::setw(7) << std::fixed << std::setprecision(2) << elapsedMs
              << "ms" << RESET << "\n";
}

// Counts visible characters rather than bytes, so multi-byte UTF-8
// sequences (e.g. the middle dot used in stats()) don't inflate the
// width used for box-padding calculations.
inline size_t visualWidth(const std::string& s) {
    size_t count = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80)
            ++count; // skip UTF-8 continuation bytes
    }
    return count;
}

// One row in a printStatBox() summary: a label and a value, tracked
// both plainly (for width/padding math) and with color codes applied
// (what actually gets printed) since ANSI escapes would otherwise
// throw off the character count.
struct StatRow {
    std::string label;
    std::string plainValue;
    std::string coloredValue;
};

// Builds a StatRow from a label, value, and the color to apply to it.
inline StatRow makeStatRow(std::string label, std::string plainValue, const char* color) {
    std::string coloredValue = std::string(color) + plainValue + RESET;
    return {std::move(label), std::move(plainValue), std::move(coloredValue)};
}

// Prints a set of label/value rows inside a boxed border, with columns
// aligned across all rows.
inline void printStatBox(const std::vector<StatRow>& rows) {
    // labelWidth includes the trailing ':' that's appended to each label.
    size_t labelWidth = 0, valueWidth = 0;
    for (const auto& r : rows) {
        labelWidth = std::max(labelWidth, visualWidth(r.label) + 1);
        valueWidth = std::max(valueWidth, visualWidth(r.plainValue));
    }

    // Extra breathing room on both sides of the box; bump this up for a
    // bigger box.
    const size_t horizontalPadding = 2;
    // padding + label(+colon) + 1-space gap + value + padding
    size_t innerWidth = labelWidth + valueWidth + 2 * horizontalPadding + 1;

    std::string rule;
    for (size_t i = 0; i < innerWidth; ++i)
        rule += "\u2500";
    std::string pad(horizontalPadding, ' ');

    std::cout << CYAN << "\u250c" << rule << "\u2510" << RESET << "\n";
    for (const auto& r : rows) {
        std::string labelWithColon = r.label + ":";
        size_t labelPad = labelWidth - visualWidth(labelWithColon);
        size_t valuePad = valueWidth - visualWidth(r.plainValue);

        std::cout << CYAN << "\u2502" << RESET << pad << BOLD << labelWithColon << RESET
                  << std::string(labelPad, ' ') << " " << std::string(valuePad, ' ')
                  << r.coloredValue << pad << CYAN << "\u2502" << RESET << "\n";
    }
    std::cout << CYAN << "\u2514" << rule << "\u2518" << RESET << "\n";
}

// Prints a boxed summary, one stat per line: total, passed, failed,
// pass rate, and (if given) elapsed time. Numbers are colored so a
// failing run is visually obvious at a glance.
inline void stats(double elapsedMs = 0.0) {
    double rate = total > 0 ? (100.0 * static_cast<double>(pass) / total) : 0.0;
    const char* rateColor = (fail == 0) ? GREEN : (pass == 0 ? RED : YELLOW);

    std::vector<StatRow> rows;
    rows.push_back(makeStatRow("Total", std::to_string(total), RESET));
    rows.push_back(makeStatRow("Passed", std::to_string(pass), GREEN));
    rows.push_back(makeStatRow("Failed", std::to_string(fail), RED));

    std::ostringstream ratePlain;
    ratePlain << std::fixed << std::setprecision(0) << rate << "%";
    rows.push_back(makeStatRow("Rate", ratePlain.str(), rateColor));

    if (elapsedMs > 0.0) {
        std::ostringstream timePlain;
        timePlain << std::fixed << std::setprecision(1) << elapsedMs << "ms";
        rows.push_back(makeStatRow("Time", timePlain.str(), DIM));
    }

    printStatBox(rows);
}