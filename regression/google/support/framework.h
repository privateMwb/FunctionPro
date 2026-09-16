#pragma once

// Single include that pulls in the entire regression-tool framework —
// the tool's main file includes only this to get the library under
// test, benchmark-snapshot parsing, comparison output, and result
// export.

// clang-format off
#include "helpers.h"  // benchmark/baseline data types, snapshot loading, comparison output
#include "export.h"   // exportJson(), exportMarkdown()
// clang-format on

// ── Run drivers ─────────────────────────────────────────────────────
//
// These are the entry points google_regression_main.cpp calls
// directly. Kept here rather than in helpers.h so helpers.h stays a
// set of reusable building blocks, and the top-level "run the whole
// comparison" / "list what's available" orchestration lives in one
// place.

// The one function to edit when retargeting this skeleton at a
// different library: sets the display label used in the exported
// markdown report's title. Called once at startup, before the tool
// runs.
inline void setProjectLabels() {
    setCustom("FunctionPro"); // TODO: replace with the library's display name
}

// Iterates baseline results, printing/recording a comparison row
// against current results for each entry, grouped and re-headered by
// baseName() -- this tool's equivalent of the custom suite's explicit
// "suite" grouping, since Google Benchmark names carry no such field
// of their own (e.g. "Insert_Impl" and "Insert_Std" both re-header
// under "Insert").
inline void printRegression(const std::vector<BenchmarkResult>& baseline,
                            const std::vector<BenchmarkResult>& current) {
    std::string currentGroup = " ";

    for (std::size_t i = 0; i < baseline.size(); ++i) {
        std::string group = baseName(baseline[i].name);

        if (currentGroup != group) {
            if (currentGroup != " ")
                endTable();
            std::cout << "\n";
            setHeader(group);
            currentGroup = group;
        }

        double bns = baseline[i].real_time;
        double cns = getCns(current, baseline[i].name);

        printComparisonRow(baseline[i].name, bns, cns);
    }

    if (currentGroup != " ")
        endTable();
}

// Prints a boxed summary of every comparison row from this run: how
// many the current side won, the baseline side won, or tied (within
// NOISE_THRESHOLD), each with a share bar -- matching the custom
// suite's own boxed summary exactly, including coloring each row by
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

// Prints every available baseline version and the benchmarks it
// recorded, oldest version first, or -- when versionFilter is
// non-empty -- just the single baseline matching that version tag
// (e.g. "v1.0.0"). A small cyan box-drawn border wraps just the
// "Version"/"Methods" header; below it, unboxed, one section per
// starting letter (with its own count) lists that letter's benchmark
// names as a small tree, each prefixed with an aligned [LetterN] id
// (e.g. "[A3]") -- same scheme as the custom suite's own [C1]/[I2]
// -style ids, just keyed by first letter instead of category. Box
// border and tree connectors are cyan, header labels white and their
// values green, letters and ids green, letter counts dark gray,
// benchmark names plain white. The letter grouping is based purely on
// spelling (not any specific library's naming scheme), so it looks the
// same regardless of what's being tested. Throws if versionFilter
// doesn't match any known baseline.
inline void printMethods(const std::string& versionFilter = "") {
    std::vector<Baseline> baselines;

    for (const auto& entry : fs::directory_iterator("benchmarks/baselines")) {
        if (!entry.is_directory())
            continue;

        if (!isGoogleBaselineName(entry.path()))
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
        std::string file = "benchmarks/baselines/" + baseline.name + "/g" + baseline.name + ".json";
        auto results = loadResults(file);
        auto names = uniqueNames(results);
        auto groups = groupMethodsByLetter(names);

        // ── Boxed header: just Version + Methods ──
        std::ostringstream versionLine, methodsLine;
        versionLine << std::left << std::setw(13) << "Version" << baseline.name;
        methodsLine << std::left << std::setw(13) << "Methods" << names.size();

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
        printBoxRow("Methods", std::to_string(names.size()), displayWidth(methodsLine.str()));

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

                std::cout << CYAN << childBranch << RESET << GREEN << "[" << id << "]" << RESET
                          << "  " << WHITE << members[j] << RESET << "\n";
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
// inside a cyan box, as "Version: N" with N in green. Only considers
// this tool's "v"-prefixed version folders that actually contain a
// "g"-prefixed Google Benchmark snapshot (e.g.
// benchmarks/baselines/v1.2.0/gv1.2.0.json) -- the custom suite's own
// "<tag>.json" snapshot lives inside these same folders but uses an
// incompatible JSON schema, so the folder is only counted here if this
// tool's own file is actually present.
inline void printList() {
    std::vector<Baseline> baselines;

    for (const auto& entry : fs::directory_iterator("benchmarks/baselines")) {
        if (!entry.is_directory())
            continue;

        if (!isGoogleBaselineName(entry.path()))
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
