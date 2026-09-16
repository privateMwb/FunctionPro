#pragma once

// Single include that pulls in the entire example framework — suite
// files include only this to get the library under test, example
// macros, output helpers, and suite registration.

// clang-format off
#include <FunctionPro/Function.h>            // FunctionPro::Function — SBO callable wrapper under example
#include <FunctionPro/MoveOnlyFunction.h>    // FunctionPro::MoveOnlyFunction — move-only variant under example
#include <FunctionPro/FunctionRef.h>         // FunctionPro::FunctionRef — non-owning callable view under example

#include "helpers.h"    // output formatting & printing helpers
#include "macros.h"     // REGISTER_EXAMPLE_SUITE()
#include "registry.h"   // ExampleSuite, ExampleRegistrar, example_registry()

#include <chrono>    // std::chrono::steady_clock (run timing)
#include <iomanip>   // std::setw, std::setprecision, std::fixed
#include <sstream>   // std::ostringstream (formatting elapsed time)
// clang-format on

// ── Suite drivers ───────────────────────────────────────────────────
//
// These build on registry.h (example_registry()) and helpers.h
// (box-drawing helpers) — kept here rather than in helpers.h so
// that dependency only flows one way: this file already includes both
// before any of the below is defined.

// Prints one example's section header as a small open-sided box: id
// and name on the top line, category dimmed underneath -- lighter
// than a fully-closed box (no right-hand border), since this frames
// the example's own printed output that immediately follows it,
// rather than standalone boxed content like `list`'s count.
inline void printExampleHeader(const ExampleSuite& suite) {
    const std::string idPrefix = "[" + suite.id + "] ";
    const std::string idName = idPrefix + suite.name;
    const int width = std::max(visualWidth(idName), visualWidth(suite.category)) + 2;

    std::cout << "\n";
    drawBorder(TL, TM, TR, {width});

    std::cout << GRAY << V << RESET << " " << YELLOW << idPrefix << RESET << BOLD
              << padCell(suite.name, width - 2 - visualWidth(idPrefix)) << RESET << " " << GRAY << V
              << RESET << "\n";

    std::cout << GRAY << V << RESET << " " << DIM << padCell(suite.category, width - 2) << RESET
              << " " << GRAY << V << RESET << "\n";

    drawBorder(BL, BM, BR, {width});
}

// Prints a boxed summary after running one or more suites: how many
// examples ran, how many categories they spanned, and how long it
// took -- same label/value box style as `list`'s Suites/Categories
// count.
inline void printRunSummary(std::size_t exampleCount, std::size_t categoryCount, double elapsedMs) {
    std::ostringstream timeStream;
    timeStream << std::fixed << std::setprecision(1) << elapsedMs << "ms";

    struct Row {
        std::string label;
        std::string value;
    };
    const std::vector<Row> rows = {
        {"Examples", std::to_string(exampleCount)},
        {"Categories", std::to_string(categoryCount)},
        {"Time", timeStream.str()},
    };

    std::size_t labelWidth = 0, valueWidth = 0;
    for (const auto& row : rows) {
        labelWidth = std::max(labelWidth, static_cast<std::size_t>(visualWidth(row.label)));
        valueWidth = std::max(valueWidth, static_cast<std::size_t>(visualWidth(row.value)));
    }

    const int boxW = static_cast<int>(labelWidth + valueWidth + 3); // padding on both sides

    std::cout << "\n";
    drawBorder(TL, TM, TR, {boxW});
    for (const auto& row : rows) {
        std::string valuePad(valueWidth - visualWidth(row.value), ' ');
        std::cout << GRAY << V << RESET << " " << padCell(row.label, static_cast<int>(labelWidth))
                  << " " << valuePad << GREEN << row.value << RESET << " " << GRAY << V << RESET
                  << "\n";
    }
    drawBorder(BL, BM, BR, {boxW});
    std::cout << "\n";
}

// Runs a chosen set of suites (already resolved by the caller -- the
// registry, or every suite in one category), printing a boxed header
// before each one's own output, then a timed run summary at the end.
// `categoryCount` is just carried through to the summary display (1
// for a category run, the true count of distinct categories for a
// full run) -- this function doesn't compute it, since the caller
// already knows which of those two cases it's in.
inline void runSuites(const std::vector<const ExampleSuite*>& suites, std::size_t categoryCount) {
    const auto start = std::chrono::steady_clock::now();

    for (const auto* suite : suites) {
        printExampleHeader(*suite);
        suite->run();
    }

    const double elapsedMs =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

    printRunSummary(suites.size(), categoryCount, elapsedMs);
}

// Runs every registered suite in order, printing a boxed header
// before each one's own output, then a timed run summary at the end.
inline void printAllExampleSuite() {
    std::vector<const ExampleSuite*> all;
    std::vector<std::string> categories;

    for (const auto& suite : example_registry()) {
        all.push_back(&suite);
        if (std::find(categories.begin(), categories.end(), suite.category) == categories.end())
            categories.push_back(suite.category);
    }

    runSuites(all, categories.size());
}

// Prints every registered suite as a tree grouped by category, with
// a boxed suite/category total at the end -- same style as the test
// and benchmark tools' own suite-list output.
inline void printExampleSuiteList() {
    std::cout << "\n" << BOLD << "Available example suites" << RESET << "\n";

    // Group suites by category, preserving registry (registration) order.
    std::vector<std::pair<std::string, std::vector<const ExampleSuite*>>> grouped;
    for (const auto& suite : example_registry()) {
        if (grouped.empty() || grouped.back().first != suite.category)
            grouped.push_back({suite.category, {}});
        grouped.back().second.push_back(&suite);
    }

    // Align the [ID] column across the whole tree, not just per category.
    std::size_t idWidth = 0;
    for (const auto& suite : example_registry())
        idWidth = std::max(idWidth, suite.id.size());

    for (const auto& [category, suites] : grouped) {
        std::cout << "\n"
                  << BOLD << CYAN << prettify(category) << RESET << DIM << " (" << suites.size()
                  << ")" << RESET << "\n";

        for (std::size_t i = 0; i < suites.size(); ++i) {
            bool last = (i + 1 == suites.size());
            std::cout << CYAN << (last ? "└─ " : "├─ ") << RESET << GREEN << "[" << std::left
                      << std::setw(static_cast<int>(idWidth)) << suites[i]->id << "]" << RESET
                      << "  " << suites[i]->name << "\n";
        }
    }

    std::cout << "\n";

    // ── Boxed Suites/Categories count -- label/value columns aligned
    // the same way as the test/benchmark tools' own count box ──
    struct CountRow {
        std::string label;
        std::string value;
    };
    const std::vector<CountRow> counts = {
        {"Suites", std::to_string(example_registry().size())},
        {"Categories", std::to_string(grouped.size())},
    };

    std::size_t labelWidth = 0, valueWidth = 0;
    for (const auto& row : counts) {
        labelWidth = std::max(labelWidth, static_cast<std::size_t>(visualWidth(row.label)));
        valueWidth = std::max(valueWidth, static_cast<std::size_t>(visualWidth(row.value)));
    }

    const int boxW = static_cast<int>(labelWidth + valueWidth + 3); // padding on both sides

    drawBorder(TL, TM, TR, {boxW});
    for (const auto& row : counts) {
        std::string valuePad(valueWidth - visualWidth(row.value), ' ');
        std::cout << GRAY << V << RESET << " " << padCell(row.label, static_cast<int>(labelWidth))
                  << " " << valuePad << GREEN << row.value << RESET << " " << GRAY << V << RESET
                  << "\n";
    }
    drawBorder(BL, BM, BR, {boxW});

    std::cout << "\n";
}

// Prints the CLI's usage summary: one aligned command/description
// table, followed by a case-insensitivity note -- same dense,
// unboxed style as the test and benchmark tools' own -h output.
// Command literals are cyan, subcommands/flags yellow, placeholders
// and descriptions dimmed, so it reads as a quick reference rather
// than a walkthrough.
inline void printUsage() {
    struct Segment {
        const char* color;
        std::string text;
    };
    struct Row {
        std::vector<Segment> command;
        std::string description;
    };

    const std::vector<Row> rows = {
        {{{CYAN, "examples"}}, "run every example in order"},
        {{{CYAN, "examples"}, {YELLOW, " list"}}, "list all examples, grouped by category"},
        {{{CYAN, "examples"}, {GRAY, " <category>"}},
         "run every example in a category, e.g. quickstart"},
        {{{CYAN, "examples"}, {GRAY, " <id>"}}, "run a single example by id, e.g. Q1"},
        {{{CYAN, "examples"}, {GRAY, " <name>"}},
         "run a single example by file name, e.g. basic_behavior"},
        {{{CYAN, "examples"}, {YELLOW, " -h | --help"}}, "show this help message"},
    };

    std::size_t colWidth = 0;
    for (const auto& row : rows) {
        std::size_t len = 0;
        for (const auto& seg : row.command)
            len += static_cast<std::size_t>(visualWidth(seg.text));
        colWidth = std::max(colWidth, len);
    }
    colWidth += 3;

    std::cout << "\n" << BOLD << "Usage:" << RESET << "\n";

    for (const auto& row : rows) {
        std::size_t len = 0;
        std::cout << "  ";
        for (const auto& seg : row.command) {
            std::cout << seg.color << seg.text << RESET;
            len += static_cast<std::size_t>(visualWidth(seg.text));
        }
        std::cout << std::string(colWidth - len, ' ') << DIM << row.description << RESET << "\n";
    }

    std::cout << "\n" << DIM << "Matching is case-insensitive." << RESET << "\n\n";
}

// Runs a single suite under its own boxed header. No closing summary
// box -- that's only useful once more than one suite ran together
// (printAllExampleSuite(), runSuites()); a single exact-match run
// already tells you everything the summary would.
inline void printOneSuite(const ExampleSuite& suite) {
    printExampleHeader(suite);
    suite.run();
    std::cout << "\n";
}