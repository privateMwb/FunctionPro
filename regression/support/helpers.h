#pragma once

// clang-format off
#include <nlohmann/json.hpp>   // nlohmann::json (loadResults — parses benchmark snapshots)

#include <algorithm>           // std::sort (printList)
#include <filesystem>          // std::filesystem::path, std::filesystem::directory_iterator
#include <fstream>             // std::ifstream (loadResults)
#include <iomanip>             // std::setw, std::setprecision, std::fixed, std::left, std::showpos
#include <iostream>            // std::cout
#include <sstream>             // std::ostringstream (printComparisonRow), std::stringstream (parseBaseline)
#include <stdexcept>           // std::runtime_error
#include <string>              // std::string, std::string_view
#include <vector>              // std::vector
// clang-format on

using json = nlohmann::json;

namespace fs = std::filesystem;

// ── Constants ───────────────────────────────────────────────────────

// ANSI terminal color codes.
inline constexpr const char* RESET = "\033[0m";
inline constexpr const char* GREEN = "\033[92m";
inline constexpr const char* RED = "\033[91m";
inline constexpr const char* YELLOW = "\033[93m";
inline constexpr const char* CYAN = "\033[96m";
inline constexpr const char* GRAY = "\033[37m";
inline constexpr const char* BLUE = "\033[94m";

// ── Data structures ─────────────────────────────────────────────────

// Shape of one entry loaded from a benchmark JSON snapshot
// (benchmarks/baselines/*.json or benchmark_results.json).
struct BenchmarkResult {
    std::string suite;
    std::string operation;
    std::uint64_t total_ns;
    std::size_t iterations;
    double ns_per_op;
};

// Shape of one row this tool produces: a baseline-vs-current comparison,
// not a raw benchmark result. Kept separate from BenchmarkResult since
// they're different data (input vs. output of this tool).
struct RegressionRow {
    std::string suite;
    std::string operation;
    std::size_t iteration;
    double baseline_ns;
    double current_ns;
    double pct_change;
};

// One parsed baseline snapshot filename (e.g. "v1.2.0.json" -> 1.2.0),
// ordered by semantic version so the newest baseline can be found.
struct Baseline {
    int major{};
    int minor{};
    int patch{};
    std::string name;

    auto operator<=>(const Baseline&) const = default;
};

// ── Run state ───────────────────────────────────────────────────────

// Display label used in the exported markdown report's title. Set via
// setCustom() before the tool runs.
inline std::string custom;

// Sets the display label used in the exported markdown report's title.
inline void setCustom(std::string name) {
    custom = name;
}

// Process-wide accumulator of comparison rows, populated by
// printComparisonRow() and read back by exportJson().
inline std::vector<RegressionRow>& regression_results() {
    static std::vector<RegressionRow> results;
    return results;
}

// Accumulates a markdown-formatted transcript of the run, built up by
// setHeader()/printComparisonRow() alongside their normal stdout printing.
// Same rationale as the benchmarks tool's markdown_buffer(): section
// grouping only exists at print time, so it's captured there rather than
// reconstructed later from regression_results().
inline std::string& markdown_buffer() {
    static std::string buffer;
    return buffer;
}

// ── Loading / parsing ───────────────────────────────────────────────

// Loads a benchmark JSON snapshot (see BenchmarkResult) from disk into a Json.
inline std::vector<BenchmarkResult> loadResults(const std::string& file) {
    std::ifstream in(file);

    if (!in) {
        throw std::runtime_error("Failed to open: " + file);
    }

    json data;
    in >> data;

    std::vector<BenchmarkResult> results;

    for (const auto& entry : data) {
        results.push_back({entry["suite"].get<std::string>(), entry["operation"].get<std::string>(),
                           entry["total_ns"].get<std::uint64_t>(),
                           entry["iterations"].get<std::size_t>(),
                           entry["ns_per_op"].get<double>()});
    }

    return results;
}

// Looks up the current ns/op for a given operation + iteration count.
inline double getCns(const std::vector<BenchmarkResult>& results, const std::string& op,
                     std::size_t iter) {
    for (const auto& result : results) {
        if (result.operation == op && result.iterations == iter) {
            return result.ns_per_op;
        }
    }

    return 0.0;
}

// Formats an iteration count (10'000/100'000/1'000'000) as a short label ("10K"/"100K"/"1M").
inline std::string convertIter(std::size_t iter) {
    return iter == 10'000 ? "10K" : iter == 100'000 ? "100K" : "1M";
}

// Parses a baseline snapshot's filename (e.g. "v1.2.0.json") into a
// comparable Baseline. Accepts an optional leading 'v'/'V'.
inline Baseline parseBaseline(const fs::path& path) {
    std::string name = path.stem().string();

    std::string version = name;
    if (!version.empty() && (version.front() == 'v' || version.front() == 'V'))
        version.erase(0, 1);

    Baseline baseline;
    baseline.name = name;

    char dot1, dot2;
    std::stringstream ss(version);

    if (!(ss >> baseline.major >> dot1 >> baseline.minor >> dot2 >> baseline.patch) ||
        dot1 != '.' || dot2 != '.') {
        throw std::runtime_error("Invalid version: " + name);
    }

    return baseline;
}

// Returns the path to the newest baseline snapshot in benchmarks/baselines.
inline std::string latestBaseline() {
    bool found = false;
    Baseline latest;
    fs::path latestPath;

    for (const auto& entry : fs::directory_iterator("benchmarks/baselines")) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
            continue;

        Baseline current = parseBaseline(entry.path());

        if (!found || current > latest) {
            latest = current;
            latestPath = entry.path();
            found = true;
        }
    }

    if (!found)
        throw std::runtime_error("No baseline snapshots found.");

    return latestPath.string();
}

// ── Output / printing ───────────────────────────────────────────────

// Prints a horizontal separator line.
inline void borderLine() {
    std::cout << GRAY << std::string(90, '-') << RESET << "\n";
}

// Prints a section header row and starts the matching markdown table.
inline void setHeader(std::string_view header) {
    borderLine();
    std::cout << std::left << CYAN << std::setw(30) << header << std::setw(15) << "Iteration"
              << std::setw(15) << "Current" << std::setw(15) << "Baseline" << std::setw(15) << "  Δ"
              << RESET << "\n";
    borderLine();

    markdown_buffer() += "\n## " + std::string(header) + "\n\n";
    markdown_buffer() += "| Test | Iteration | Current | Baseline | Δ |\n";
    markdown_buffer() += "|---|---|---|---|---|\n";
}

// Prints (and records, for export) one baseline-vs-current comparison
// row. bNs is the baseline ns/op, cNs is the current run's ns/op —
// printed under the "Current"/"Baseline" columns set up by setHeader(),
// in that order.
inline void printComparisonRow(std::string_view suite, std::string_view name, std::size_t iter,
                               std::string_view iteration, double bNs, double cNs) {
    const double pct = bNs == 0 ? 0.0 : (bNs - cNs) / cNs * 100.0;

    // 0% (no measured difference) is treated as neutral; anything else
    // colors green (faster) or red (slower) by direction.
    const char* deltaColor = (pct > 0.0) ? GREEN : (pct < 0.0) ? RED : GRAY;

    std::ostringstream deltaStream;
    deltaStream << std::showpos << std::fixed << std::setprecision(1) << pct << "%";

    std::ostringstream currentStream;
    currentStream << cNs << " ns";

    std::ostringstream baselineStream;
    baselineStream << bNs << " ns";

    // clang-format off
    std::cout << std::left 
              << std::setw(30) << name 
              << std::setw(15) << iteration 
              << std::setw(15) << currentStream.str() 
              << std::setw(15) << baselineStream.str() 
              << deltaColor << deltaStream.str() 
              << RESET << "\n";
    // clang-format on

    markdown_buffer() += "| " + std::string(name) + " | " + std::string(iteration) + " | " +
                         currentStream.str() + " | " + baselineStream.str() + " | " +
                         deltaStream.str() + " |\n";

    regression_results().push_back({std::string(suite), std::string(name), iter, bNs, cNs, pct});
}