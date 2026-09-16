#pragma once

// Single include that pulls in the entire regression-tool framework —
// the tool's main file includes only this to get the library under
// test, benchmark-snapshot parsing, comparison output, and result
// export.

// clang-format off
#include "helpers.h"   // benchmark/baseline data types, snapshot loading, comparison output
#include "export.h"    // exportJson(), exportMarkdown()
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
    setCustom("FunctionPro"); // TODO: replace with the library's display name
}

// Iterates baseline results, printing/recording a comparison row against
// current results for each entry, grouped and re-headered by suite.
inline void printRegression(const std::vector<BenchmarkResult>& baseline,
                            const std::vector<BenchmarkResult>& current) {
    std::string currentSuite = " ";

    for (std::size_t i = 0; i < baseline.size(); ++i) {
        std::string suite = baseline[i].suite;
        std::string op = baseline[i].operation;

        if (currentSuite != suite) {
            if (currentSuite != " ")
                endTable();
            std::cout << "\n";
            setHeader(suite);
            currentSuite = suite;
        }

        std::size_t iter = baseline[i].iterations;
        std::string citer = convertIter(iter);
        double bns = baseline[i].ns_per_op;
        double cns = getCns(current, op, iter);

        printComparisonRow(suite, op, iter, citer, bns, cns);
    }

    if (currentSuite != " ")
        endTable();
}

// Prints a boxed summary of every comparison row from this run: how
// many the current side won, the baseline side won, or tied (within
// NOISE_THRESHOLD), each with a share bar -- matching the benchmark
// tool's own boxed summary exactly, including coloring each row by
// the same BLUE/MAGENTA/GRAY identity used for the current/baseline
// columns in the table header, rather than a green/red good-bad
// scheme. No-op (prints nothing) if no rows were recorded. Also
// appended to the exported markdown report.
inline void printSummary() {
    int faster = 0;
    int slower = 0;
    int tied = 0;

    for (const auto& row : regression_results()) {
        if (std::fabs(row.pct_change) < NOISE_THRESHOLD)
            ++tied;
        else if (row.pct_change > 0.0)
            ++faster;
        else
            ++slower;
    }

    const int total = faster + slower + tied;
    if (total == 0)
        return;

    const int labelW = 20;
    const int valueW = 26;

    std::cout << "\n";
    drawBorder(TL, TM, TR, {labelW, valueW});

    std::cout << GRAY << V << RESET << BOLD << CYAN << padCell(" Summary", labelW) << RESET << GRAY
              << V << RESET << padCell(" " + std::to_string(total) + " comparisons", valueW) << GRAY
              << V << RESET << "\n";

    drawBorder(ML, MM, MR, {labelW, valueW});

    auto row = [&](const char* color, const std::string& label, int count) {
        const double share = static_cast<double>(count) / static_cast<double>(total) * 100.0;

        std::ostringstream countPct;
        countPct << count << " (" << std::fixed << std::setprecision(1) << share << "%)";

        const std::string val = " " + padCell(countPct.str(), 12) + " " + bar(share, 100.0);

        std::cout << GRAY << V << RESET << color << padCell(" " + label, labelW) << RESET << GRAY
                  << V << RESET << color << padCell(val, valueW) << RESET << GRAY << V << RESET
                  << "\n";

        markdown_buffer() += "| " + label + " | " + std::to_string(count) + " (" +
                             std::to_string(static_cast<int>(share)) + "%) |\n";
    };

    markdown_buffer() += "\n## Summary\n\n";
    markdown_buffer() += "| Result | Count |\n|---|---|\n";

    row(BLUE, currentLabel + " faster", faster);
    row(MAGENTA, baselineLabel + " faster", slower);
    row(GRAY, "Tie", tied);

    drawBorder(BL, BM, BR, {labelW, valueW});
    std::cout << "\n";
}

// Prints every available baseline version and the benchmark methods
// (operations) it recorded, oldest version first, or -- when
// versionFilter is non-empty -- just the single baseline matching
// that version tag (e.g. "v1.0.0"). A small cyan box-drawn border
// wraps just the "Version"/"Methods" header; below it, unboxed, one
// section per starting letter (with its own method count) lists that
// letter's methods as a small tree, each prefixed with an aligned
// [LetterN] id (e.g. "[P3]") -- same scheme as the test suite's own
// [C1]/[I2]-style ids, just keyed by first letter instead of
// category. Box border and tree connectors are cyan, header labels
// white and their values green, letters and ids green, letter counts
// dark gray, method names plain white. The letter grouping is based
// purely on spelling (not any specific benchmark's naming scheme), so
// it looks the same regardless of what's being tested. Throws if
// versionFilter doesn't match any known baseline.
inline void printMethods(const std::string& versionFilter = "") {
    std::vector<Baseline> baselines;

    for (const auto& entry : fs::directory_iterator("benchmarks/baselines")) {
        if (!entry.is_directory())
            continue;

        if (!isCustomBaselineName(entry.path()))
            continue;

        baselines.push_back(parseBaseline(entry.path()));
    }

    std::sort(baselines.begin(), baselines.end());

    if (!versionFilter.empty()) {
        auto it = std::find_if(baselines.begin(), baselines.end(),
                               [&](const Baseline& b) { return b.name == versionFilter; });

        if (it == baselines.end())
            throw std::runtime_error("No baseline found for version '" + versionFilter + "'.");

        baselines = {*it};
    }

    for (const auto& baseline : baselines) {
        std::string file = "benchmarks/baselines/" + baseline.name + "/" + baseline.name + ".json";
        auto results = loadResults(file);
        auto operations = uniqueOperations(results);
        auto groups = groupMethodsByLetter(operations);

        // ── Boxed header: just Version + Methods ──
        std::ostringstream versionLine, methodsLine;
        versionLine << std::left << std::setw(13) << "Version" << baseline.name;
        methodsLine << std::left << std::setw(13) << "Methods" << operations.size();

        std::size_t headerWidth =
            std::max(displayWidth(versionLine.str()), displayWidth(methodsLine.str()));

        auto printBoxRow = [&](const std::string& label, const std::string& value,
                               std::size_t plainLen) {
            std::size_t pad = headerWidth - plainLen;
            std::cout << CYAN << "│ " << RESET << WHITE << std::left << std::setw(13) << label
                      << RESET << GREEN << value << RESET << std::string(pad, ' ') << CYAN << " │"
                      << RESET << "\n";
        };

        std::cout << CYAN << "┌";
        for (std::size_t i = 0; i < headerWidth + 2; ++i)
            std::cout << "─";
        std::cout << "┐" << RESET << "\n";

        printBoxRow("Version", baseline.name, displayWidth(versionLine.str()));
        printBoxRow("Methods", std::to_string(operations.size()), displayWidth(methodsLine.str()));

        std::cout << CYAN << "└";
        for (std::size_t i = 0; i < headerWidth + 2; ++i)
            std::cout << "─";
        std::cout << "┘" << RESET << "\n\n";

        // ── Unboxed letter tree ──

        for (std::size_t i = 0; i < groups.size(); ++i) {
            const auto& [letter, members] = groups[i];

            std::cout << GREEN << letter << RESET << " " << DARK_GRAY << "(" << members.size()
                      << ")" << RESET << "\n";

            // Align the [ID] column within this letter group only --
            // e.g. group A's ids stay at their own natural width even
            // though group W (12 members) needs a wider one.
            const std::size_t idWidth = std::to_string(members.size()).size() + 1;

            for (std::size_t j = 0; j < members.size(); ++j) {
                bool lastMember = (j + 1 == members.size());
                std::string childBranch = lastMember ? "└─ " : "├─ ";
                std::string id = std::string(1, letter) + std::to_string(j + 1);

                std::cout << CYAN << childBranch << RESET << GREEN << "[" << std::left
                          << std::setw(static_cast<int>(idWidth)) << id << "]" << RESET << "  "
                          << WHITE << members[j] << RESET << "\n";
            }

            if (i + 1 != groups.size())
                std::cout << "\n";
        }

        std::cout << "\n";
    }
}

// Prints every available baseline folder's version tag, sorted oldest
// to newest. "Available version" (white) and the project's display
// name (the `custom` value set via setCustom(), in cyan) print plain,
// unboxed; below them, also unboxed, a small cyan tree (├─/└─) lists
// each version tag in green. Only the trailing baseline count sits
// inside a cyan box, as "Version: N" with N in green -- previously
// shown as a "· N baselines" suffix on the title, it now lives at the
// bottom instead. Only considers this tool's "v"-prefixed version
// folders (e.g. benchmarks/baselines/v1.2.0/) -- the
// google_regressions tool's "gv"-prefixed snapshot lives inside these
// same folders but uses an incompatible JSON schema, so only the
// folder name itself is inspected here, not its contents.
inline void printList() {
    std::vector<Baseline> baselines;

    for (const auto& entry : fs::directory_iterator("benchmarks/baselines")) {
        if (!entry.is_directory())
            continue;

        if (!isCustomBaselineName(entry.path()))
            continue;

        baselines.push_back(parseBaseline(entry.path()));
    }

    std::sort(baselines.begin(), baselines.end());

    // ── Unboxed title, custom value, and tree ──

    std::cout << WHITE << "Available version" << RESET << "\n\n";
    std::cout << CYAN << custom << RESET << "\n";

    for (std::size_t i = 0; i < baselines.size(); ++i) {
        bool last = (i + 1 == baselines.size());
        std::string branch = last ? "└─ " : "├─ ";
        std::cout << CYAN << branch << RESET << GREEN << baselines[i].name << RESET << "\n";
    }

    std::cout << "\n";

    // ── Boxed count ──

    const std::string countLine = "Version: " + std::to_string(baselines.size());
    const std::size_t width = displayWidth(countLine);

    std::cout << CYAN << "┌";
    for (std::size_t i = 0; i < width + 2; ++i)
        std::cout << "─";
    std::cout << "┐" << RESET << "\n";

    std::cout << CYAN << "│ " << RESET << "Version: " << GREEN << baselines.size() << RESET << CYAN
              << " │" << RESET << "\n";

    std::cout << CYAN << "└";
    for (std::size_t i = 0; i < width + 2; ++i)
        std::cout << "─";
    std::cout << "┘" << RESET << "\n";
}