#pragma once

// Single include that pulls in the entire benchmark framework — suite
// files include only this to get the library under test, benchmarking
// macros, output helpers, suite registration, and result export.

// clang-format off
#include <FunctionPro/Function.h>            // FunctionPro::Function — SBO callable wrapper under benchmark
#include <FunctionPro/MoveOnlyFunction.h>    // FunctionPro::MoveOnlyFunction — move-only variant under benchmark
#include <FunctionPro/FunctionRef.h>         // FunctionPro::FunctionRef — non-owning callable view under benchmark

#include "export.h"     // exportJson(), exportMarkdown()
#include "helpers.h"    // output formatting & printing helpers
#include "macros.h"     // BENCH/BENCH_SOLO/BENCH_CUSTOM, REGISTER_BENCH_SUITE()
#include "registry.h"   // BenchSuite, BenchRegistrar, bench_registry()
#include "reference.h"  //
// clang-format on

// ── Project configuration ───────────────────────────────────────────

// The one function to edit when retargeting this skeleton at a
// different library: sets the display label for the custom
// implementation under test and the reference implementation it's
// compared against. Called once at startup, before any suite runs.
inline void setProjectLabels() {
    setCustom("FunctionPro");
    setStandard("stdFunction");
}

// ── Suite drivers ───────────────────────────────────────────────────
//
// These build on registry.h (bench_registry()), export.h (exportJson(),
// exportMarkdown()), and helpers.h (setHeader(), setSuite()) — kept here
// rather than in helpers.h so that dependency only flows one way: this
// file already includes all three before any of the below is defined.

// Prints every registered suite's category header, one at a time, then
// a summary of wins/losses across the whole run, then exports the
// accumulated results as JSON and markdown.
inline void printAllBenchSuite() {
    for (const auto& suite : bench_registry()) {
        std::cout << "\n";
        beginSuite(suite.name);
        suite.run();
        closeTable();
    }
    printSummary();
    exportJson("benchmark_results.json");
    exportMarkdown("benchmark_results.md");
}

// Lists every registered suite as a tree grouped by category, with
// a boxed suite/category total at the end -- same style as the test
// tool's own printTestSuiteList().
inline void printBenchSuiteList() {
    std::cout << "\n" << BOLD << "Available bench suites" << RESET << "\n";

    // Group suites by category, preserving registry (registration) order.
    std::vector<std::pair<std::string, std::vector<const BenchSuite*>>> grouped;
    for (const auto& suite : bench_registry()) {
        if (grouped.empty() || grouped.back().first != suite.category)
            grouped.push_back({suite.category, {}});
        grouped.back().second.push_back(&suite);
    }

    // Align the [ID] column across the whole tree, not just per category.
    std::size_t idWidth = 0;
    for (const auto& suite : bench_registry())
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
    // the same way as the test tool's printStatBox() ──
    struct CountRow {
        std::string label;
        std::string value;
    };
    const std::vector<CountRow> counts = {
        {"Suites", std::to_string(bench_registry().size())},
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
// unboxed style as the regression and test tools' own -h output.
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
        {{{CYAN, "benchmarks"}}, "run every suite, print tables, export results"},
        {{{CYAN, "benchmarks"}, {YELLOW, " list"}}, "list all suites, grouped by category"},
        {{{CYAN, "benchmarks"}, {GRAY, " <category>"}},
         "run every suite in a category, e.g. search"},
        {{{CYAN, "benchmarks"}, {GRAY, " <id>"}}, "run a single suite by id, e.g. S1"},
        {{{CYAN, "benchmarks"}, {GRAY, " <name>"}},
         "run a single suite by file name, e.g. binary_search"},
        {{{CYAN, "benchmarks"}, {YELLOW, " -h | --help"}}, "show this help message"},
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

// Runs a single suite (matched by name, id, or category by the caller)
// and prints its table. Does not export — callers that use this run it
// per matching suite, then export once after the loop.
inline void printOneSuite(const BenchSuite& suite) {
    std::cout << "\n";
    beginSuite(suite.name);
    suite.run();
    closeTable();
}