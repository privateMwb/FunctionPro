#pragma once

// Single include that pulls in the entire regression-tool framework —
// the tool's main file includes only this to get the library under
// test, benchmark-snapshot parsing, comparison output, and result
// export.

// clang-format off
#include <FunctionPro/Function.h>            // core type under test — not directly used by this tool's own
#include <FunctionPro/MoveOnlyFunction.h>    // code today; keep only if the main file needs it, otherwise drop
#include <FunctionPro/FunctionRef.h>        

#include "helpers.h"            // benchmark/baseline data types, snapshot loading, comparison output
#include "export.h"             // exportJson(), exportMarkdown()
// clang-format on

// ── Run drivers ─────────────────────────────────────────────────────
//
// These are the entry points regression_main.cpp calls directly. Kept
// here rather than in helpers.h so helpers.h stays a set of reusable
// building blocks, and the top-level "run the whole comparison" /
// "list what's available" orchestration lives in one place.

// The one function to edit when retargeting this skeleton at a
// different library: sets the display label used in the exported
// markdown report's title. Called once at startup, before the tool
// runs.
inline void setProjectLabels() {
    setCustom("FunctionPro");
}

// Iterates baseline results, printing/recording a comparison row against
// current results for each entry, grouped and re-headered by suite.
inline void printRegression(const std::vector<BenchmarkResult>& baseline,
                            const std::vector<BenchmarkResult>& current) {
    std::string currentSuite = " ";

    for (std::size_t i = 0; i < baseline.size(); ++i) {
        std::string suite = baseline[i].suite;

        if (currentSuite != suite) {
            std::cout << "\n";
            setHeader(suite);
            currentSuite = suite;
        }

        std::string op = baseline[i].operation;
        std::size_t iter = baseline[i].iterations;
        std::string citer = convertIter(iter);
        double bns = baseline[i].ns_per_op;
        double cns = getCns(current, op, iter);

        printComparisonRow(suite, op, iter, citer, bns, cns);

        if (citer == "1M")
            std::cout << "\n";
    }
}

// Prints every available baseline snapshot's name, sorted oldest to newest.
inline void printList() {
    std::vector<Baseline> baselines;

    for (const auto& entry : fs::directory_iterator("benchmarks/baselines")) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
            continue;

        baselines.push_back(parseBaseline(entry.path()));
    }

    std::sort(baselines.begin(), baselines.end());

    std::cout << "Available baselines:\n\n";

    for (const auto& baseline : baselines)
        std::cout << "  " << baseline.name << '\n';
}