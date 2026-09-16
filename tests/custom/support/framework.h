#pragma once

// Single include that pulls in the entire test framework — suite
// files include only this to get the library under test, testing
// macros, output helpers, and suite registration.

// clang-format off
#include <FunctionPro/Function.h>            // FunctionPro::Function — SBO callable wrapper under test
#include <FunctionPro/MoveOnlyFunction.h>    // FunctionPro::MoveOnlyFunction — move-only variant under test
#include <FunctionPro/FunctionRef.h>         // FunctionPro::FunctionRef — non-owning callable view under test

#include "helpers.h"    // output formatting & printing helpers
#include "macros.h"     // RUN/CHK/CHK_THROWS, REGISTER_TEST_SUITE()
#include "registry.h"   // TestSuite, TestRegistrar, test_registry()

#include <chrono>    // std::chrono::steady_clock
#include <iomanip>   // std::setw
// clang-format on

// ── Suite drivers ───────────────────────────────────────────────────
//
// These build on registry.h (test_registry()) and helpers.h (setTitle(),
// stats()) — kept here rather than in helpers.h so that dependency only
// flows one way: this file already includes both before any of the
// below is defined.

// Runs every registered suite in order, then prints the overall stats,
// including total elapsed wall-clock time.
inline void printAllTestSuite() {
    auto start = std::chrono::steady_clock::now();

    for (const auto& suite : test_registry()) {
        std::cout << "\n";
        setTitle(suite.name);
        suite.run();
    }

    double elapsedMs =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

    std::cout << "\n";
    stats(elapsedMs);
    std::cout << "\n";
}

// Prints every registered suite as a tree grouped by category, with
// a boxed suite/category total at the end.
inline void printTestSuiteList() {
    std::cout << "\n" << BOLD << "Available test suites" << RESET << "\n";

    // Group suites by category, preserving registry (registration) order.
    std::vector<std::pair<std::string, std::vector<const TestSuite*>>> grouped;
    for (const auto& suite : test_registry()) {
        if (grouped.empty() || grouped.back().first != suite.category)
            grouped.push_back({suite.category, {}});
        grouped.back().second.push_back(&suite);
    }

    // Align the [ID] column across the whole tree, not just per category.
    size_t idWidth = 0;
    for (const auto& suite : test_registry())
        idWidth = std::max(idWidth, suite.id.size());

    for (const auto& [category, suites] : grouped) {
        std::cout << "\n"
                  << BOLD << CYAN << prettify(category) << RESET << DIM << " (" << suites.size()
                  << ")" << RESET << "\n";

        for (size_t i = 0; i < suites.size(); ++i) {
            bool last = (i + 1 == suites.size());
            std::cout << CYAN << (last ? "\u2514\u2500 " : "\u251c\u2500 ") << RESET << GREEN << "["
                      << std::left << std::setw(static_cast<int>(idWidth)) << suites[i]->id << "]"
                      << RESET << "  " << suites[i]->name << "\n";
        }
    }

    std::cout << "\n";

    std::vector<StatRow> rows;
    rows.push_back(makeStatRow("Suites", std::to_string(test_registry().size()), GREEN));
    rows.push_back(makeStatRow("Categories", std::to_string(grouped.size()), GREEN));
    printStatBox(rows);

    std::cout << "\n";
}

// Runs a single suite and prints its title. Does not print stats —
// callers decide when to summarize, since a category match runs this
// once per suite but only wants stats printed once, at the end. Returns
// the elapsed time in milliseconds so callers can accumulate it for
// the final summary.
inline double printOneSuite(const TestSuite& suite) {
    auto start = std::chrono::steady_clock::now();

    std::cout << "\n";
    setTitle(suite.name);
    suite.run();
    std::cout << "\n";

    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
        .count();
}

// Prints the CLI's usage summary: one aligned command/description
// table, followed by a case-insensitivity note -- same dense,
// unboxed style as the regression tools' own -h output. Command
// literals are cyan, subcommands/flags yellow, placeholders and
// descriptions dimmed, so it reads as a quick reference rather than a
// walkthrough.
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
        {{{CYAN, "tests"}}, "run every suite and print stats"},
        {{{CYAN, "tests"}, {YELLOW, " list"}}, "list all suites, grouped by category"},
        {{{CYAN, "tests"}, {GRAY, " <category>"}}, "run every suite in a category, e.g. unit"},
        {{{CYAN, "tests"}, {GRAY, " <id>"}}, "run a single suite by id, e.g. U1"},
        {{{CYAN, "tests"}, {GRAY, " <name>"}},
         "run a single suite by file name, e.g. basic_behavior"},
        {{{CYAN, "tests"}, {YELLOW, " -h | --help"}}, "show this help message"},
    };

    std::size_t colWidth = 0;
    for (const auto& row : rows) {
        std::size_t len = 0;
        for (const auto& seg : row.command)
            len += visualWidth(seg.text);
        colWidth = std::max(colWidth, len);
    }
    colWidth += 3;

    std::cout << "\n" << BOLD << "Usage:" << RESET << "\n";

    for (const auto& row : rows) {
        std::size_t len = 0;
        std::cout << "  ";
        for (const auto& seg : row.command) {
            std::cout << seg.color << seg.text << RESET;
            len += visualWidth(seg.text);
        }
        std::cout << std::string(colWidth - len, ' ') << DIM << row.description << RESET << "\n";
    }

    std::cout << "\n" << DIM << "Matching is case-insensitive." << RESET << "\n\n";
}