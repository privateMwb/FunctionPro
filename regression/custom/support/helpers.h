#pragma once

// clang-format off
#include <nlohmann/json.hpp>   // nlohmann::json (loadResults — parses benchmark snapshots)

#include <algorithm>           // std::sort (printList), std::clamp (bar)
#include <cctype>              // std::toupper, std::isalpha, std::isdigit (groupMethodsByLetter, resolveMethodId)
#include <chrono>              // nanoseconds, duration<> (formatDuration)
#include <cmath>               // std::fabs, std::lround (printComparisonRow, printSummary, bar)
#include <filesystem>          // std::filesystem::path, std::filesystem::directory_iterator
#include <fstream>             // std::ifstream (loadResults)
#include <initializer_list>    // std::initializer_list (drawBorder)
#include <iomanip>             // std::setw, std::setprecision, std::fixed, std::left, std::showpos
#include <iostream>            // std::cout
#include <sstream>             // std::ostringstream (printComparisonRow), std::stringstream (parseBaseline)
#include <stdexcept>           // std::runtime_error
#include <string>              // std::string, std::string_view
#include <vector>              // std::vector
// clang-format on

using json = nlohmann::json;

using namespace std::chrono;

namespace fs = std::filesystem;

// ── Constants ───────────────────────────────────────────────────────

// ANSI terminal color codes.
inline constexpr const char* RESET = "\033[0m";
inline constexpr const char* BOLD = "\033[1m";
inline constexpr const char* GREEN = "\033[92m";
inline constexpr const char* RED = "\033[91m";
inline constexpr const char* YELLOW = "\033[93m";
inline constexpr const char* CYAN = "\033[96m";
inline constexpr const char* GRAY = "\033[37m";
// A visibly darker gray than GRAY (which renders closer to light
// gray/white on most terminals) -- used for the per-letter method
// counts under `methods`, where GRAY read as too light.
inline constexpr const char* DARK_GRAY = "\033[90m";
inline constexpr const char* BLUE = "\033[94m";    // current column, matching the benchmark tool
inline constexpr const char* MAGENTA = "\033[95m"; // baseline column, matching the benchmark tool
// Bright white, used for method names under `methods` so they read as
// plain, undecorated text against the colored tree/letters around them.
inline constexpr const char* WHITE = "\033[97m";
// 256-color code (widely supported) for an actual sky-blue tone,
// distinct from the plain 16-color CYAN already used for suite
// headers elsewhere -- used only for method names under `methods`.
inline constexpr const char* SKY_BLUE = "\033[38;5;117m";

// Percent-change thresholds shared by row coloring and the summary
// line, so what counts as "unchanged" or "a big swing" agrees between
// the two. Anything within NOISE_THRESHOLD of 0% is treated as
// measurement noise rather than a real change; anything past
// SIGNIFICANT_THRESHOLD is emphasized in bold as a meaningfully large
// swing either direction.
inline constexpr double NOISE_THRESHOLD = 1.0;
inline constexpr double SIGNIFICANT_THRESHOLD = 10.0;

// ── Data structures ─────────────────────────────────────────────────

// Shape of one entry loaded from a benchmark JSON snapshot
// (benchmarks/baselines/<tag>/<tag>.json or benchmark_results.json).
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

// One parsed baseline version folder (e.g. "v1.2.0" -> 1.2.0),
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

// Display labels for the two comparison columns in printed output and
// the exported markdown report -- a version tag when a column shows a
// tagged baseline snapshot (e.g. "v1.2.0"), or "Current" for the live
// local benchmark run, which has no tag of its own. Set via
// setLabels() before the tool runs.
inline std::string currentLabel = "Current";
inline std::string baselineLabel = "Baseline";

// Sets the display labels used for the two comparison columns.
inline void setLabels(std::string current, std::string baseline) {
    currentLabel = std::move(current);
    baselineLabel = std::move(baseline);
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

// Returns every distinct benchmark method (operation) present in a
// results vector, in first-seen order -- used by `regression methods`
// to list what's available per baseline version.
inline std::vector<std::string> uniqueOperations(const std::vector<BenchmarkResult>& results) {
    std::vector<std::string> operations;

    for (const auto& result : results) {
        if (std::find(operations.begin(), operations.end(), result.operation) == operations.end())
            operations.push_back(result.operation);
    }

    return operations;
}

// Keeps only the entries for one benchmark method (operation),
// preserving relative order -- used to restrict a comparison to a
// single method (e.g. `regression element_access`).
inline std::vector<BenchmarkResult> filterByOperation(const std::vector<BenchmarkResult>& results,
                                                      const std::string& operation) {
    std::vector<BenchmarkResult> filtered;

    for (const auto& result : results) {
        if (result.operation == operation)
            filtered.push_back(result);
    }

    return filtered;
}

// Groups operation names by their first character (case-insensitive),
// sorted alphabetically both across groups and within each group.
// Purely a display grouping for `regression methods` -- based only on
// spelling, so it works the same regardless of what library or naming
// scheme the benchmark methods happen to belong to.
inline std::vector<std::pair<char, std::vector<std::string>>>
groupMethodsByLetter(std::vector<std::string> operations) {
    std::sort(operations.begin(), operations.end(), [](const std::string& a, const std::string& b) {
        return std::lexicographical_compare(
            a.begin(), a.end(), b.begin(), b.end(),
            [](unsigned char x, unsigned char y) { return std::toupper(x) < std::toupper(y); });
    });

    std::vector<std::pair<char, std::vector<std::string>>> groups;

    for (const auto& operation : operations) {
        char letter =
            operation.empty()
                ? '?'
                : static_cast<char>(std::toupper(static_cast<unsigned char>(operation.front())));

        if (!groups.empty() && groups.back().first == letter)
            groups.back().second.push_back(operation);
        else
            groups.push_back({letter, {operation}});
    }

    return groups;
}

// Filters both sides of a comparison down to one benchmark method in
// place, and reports (by throwing) if either side turns out not to
// have that method at all -- rather than silently comparing nothing.
// currentLabel/baselineLabel name whichever side is missing it in the
// error, so the message points at the right side (e.g. "v1.0.0" or
// "Current").
inline void applyMethodFilter(std::vector<BenchmarkResult>& baselineResults,
                              std::vector<BenchmarkResult>& currentResults,
                              const std::string& method, const std::string& currentLabel,
                              const std::string& baselineLabel) {
    baselineResults = filterByOperation(baselineResults, method);
    currentResults = filterByOperation(currentResults, method);

    if (baselineResults.empty())
        throw std::runtime_error(baselineLabel + " has no benchmark method '" + method + "'.");

    if (currentResults.empty())
        throw std::runtime_error(currentLabel + " has no benchmark method '" + method + "'.");
}

// True if a method filter argument looks like a `methods`-listing ID
// (e.g. "U1", "u1") -- exactly one letter followed by one or more
// digits and nothing else, matching the [LetterN] ids printed by
// printMethods(). A real method name in this project is always a
// multi-word/punctuated phrase, so this shape is unambiguous.
inline bool looksLikeMethodId(const std::string& s) {
    if (s.size() < 2 || !std::isalpha(static_cast<unsigned char>(s.front())))
        return false;

    for (std::size_t i = 1; i < s.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;

    return true;
}

// Resolves a `methods`-listing ID (e.g. "U1") against one baseline
// snapshot's own letter-grouped method list -- the same grouping
// printMethods() itself builds -- and returns the actual method name
// it refers to (e.g. "unsubscribe x100"). Case-insensitive on the
// letter. Throws if the letter has no group, or the index is out of
// range for that group; only call this once looksLikeMethodId() has
// already confirmed the argument's shape.
inline std::string resolveMethodId(const std::string& id, const std::string& file) {
    char letter = static_cast<char>(std::toupper(static_cast<unsigned char>(id.front())));
    std::size_t index = static_cast<std::size_t>(std::stoul(id.substr(1)));

    auto groups = groupMethodsByLetter(uniqueOperations(loadResults(file)));

    for (const auto& [groupLetter, members] : groups) {
        if (groupLetter != letter)
            continue;

        if (index == 0 || index > members.size())
            throw std::runtime_error("Method id '" + id + "' is out of range for '" +
                                     std::string(1, letter) + "' (only " +
                                     std::to_string(members.size()) + ").");

        return members[index - 1];
    }

    throw std::runtime_error("Unknown method id: '" + id + "'.");
}

// Formats an iteration count (10'000/100'000/1'000'000) as a short label ("10K"/"100K"/"1M").
inline std::string convertIter(std::size_t iter) {
    return iter == 10'000 ? "10K" : iter == 100'000 ? "100K" : "1M";
}

// Parses a baseline version folder's name (e.g. "v1.2.0") into a
// comparable Baseline. Accepts an optional leading 'v'/'V'.
inline Baseline parseBaseline(const fs::path& path) {
    std::string name = path.filename().string();

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

// Attempts to parse a raw CLI argument (not a filesystem path) as a
// "v"/"V"-prefixed semantic version, e.g. "v1.2.0". Used to tell apart
// a version tag from a benchmark method name (like "element_access")
// when reading command-line arguments -- returns false, leaving `out`
// untouched, rather than throwing, since a false result here just
// means "try this argument as something else" rather than an error.
inline bool tryParseVersionArg(const std::string& arg, Baseline& out) {
    if (arg.empty() || (arg.front() != 'v' && arg.front() != 'V'))
        return false;

    Baseline baseline;
    baseline.name = arg;

    char dot1, dot2, extra;
    std::stringstream ss(arg.substr(1));

    if (!(ss >> baseline.major >> dot1 >> baseline.minor >> dot2 >> baseline.patch) ||
        dot1 != '.' || dot2 != '.' || (ss >> extra)) {
        return false;
    }

    out = baseline;
    return true;
}

// True if a baselines-directory entry belongs to this tool -- i.e.
// it's a "v"/"V"-prefixed custom-suite version folder, not one of the
// google_regressions tool's "gv"/"GV"-prefixed snapshot files sitting
// inside these same folders (different, incompatible JSON schema). A
// "gv..." name's first character is 'g', so it's excluded by this
// check without needing special-casing.
inline bool isCustomBaselineName(const fs::path& path) {
    std::string name = path.filename().string();
    return !name.empty() && (name.front() == 'v' || name.front() == 'V');
}

// Returns the path to the newest baseline snapshot in
// benchmarks/baselines, considering only this tool's "v"-prefixed
// version folders (e.g. benchmarks/baselines/v1.2.0/v1.2.0.json).
inline std::string latestBaseline() {
    bool found = false;
    Baseline latest;
    fs::path latestDir;

    for (const auto& entry : fs::directory_iterator("benchmarks/baselines")) {
        if (!entry.is_directory())
            continue;

        if (!isCustomBaselineName(entry.path()))
            continue;

        Baseline current = parseBaseline(entry.path());

        if (!found || current > latest) {
            latest = current;
            latestDir = entry.path();
            found = true;
        }
    }

    if (!found)
        throw std::runtime_error("No baseline snapshots found.");

    return (latestDir / (latestDir.filename().string() + ".json")).string();
}

// ── Output / printing ───────────────────────────────────────────────

// Legacy plain separator, kept for anything printed outside the boxed
// tables (matches the benchmark tool's own borderLine(), kept there
// for the same reason).
inline void borderLine() {
    std::cout << GRAY << std::string(90, '-') << RESET << "\n";
}

// Counts the visible terminal columns in a UTF-8 string, treating
// each Unicode codepoint as one column. std::string::size() counts
// bytes, which overcounts anything using multi-byte characters (like
// the "├─"/"└─" tree connectors, the bar's █/░, or the Δ symbol), so
// this is needed to size a box border correctly around content that
// mixes ASCII and those glyphs.
inline std::size_t displayWidth(const std::string& text) {
    std::size_t width = 0;
    for (unsigned char c : text)
        if ((c & 0xC0) != 0x80) // not a UTF-8 continuation byte
            ++width;
    return width;
}

// Box-drawing pieces. UTF-8 multi-byte characters, so borders are built
// by repeating these tokens rather than via std::string(n, ch). Same
// glyphs and roles as the benchmark tool's own box-drawing constants.
// clang-format off
inline constexpr const char* H  = "─";
inline constexpr const char* V  = "│";
inline constexpr const char* TL = "┌"; inline constexpr const char* TM = "┬"; inline constexpr const char* TR = "┐";
inline constexpr const char* ML = "├"; inline constexpr const char* MM = "┼"; inline constexpr const char* MR = "┤";
inline constexpr const char* BL = "└"; inline constexpr const char* BM = "┴"; inline constexpr const char* BR = "┘";

// Column widths shared by the box-drawn comparison table and its
// borders -- a border segment's dash count must always match the
// padCell() width of the cell above it, so both are defined from
// these constants. Matches the benchmark tool's own widths so both
// tools' tables read the same width at a glance.
inline constexpr int TEST_W  = 26;
inline constexpr int ITER_W  = 11;
inline constexpr int VAL_W   = 24;   // duration + bar, comparison table
inline constexpr int DELTA_W = 14;   // fits large deltas like "▲ +93904.5%" without overflow
inline constexpr int BAR_W   = 10;   // characters in the mini bar chart
inline constexpr int DUR_W   = 10;   // fixed width reserved for the duration text before a
                                     // bar, so bars in the same column all start at the same
                                     // x -- otherwise "9.99 ms" vs "114.77 ms" vs "1.09 s" push
                                     // the bar to a different offset each row.
// clang-format on

// Repeats a (possibly multi-byte) UTF-8 token `n` times.
inline std::string repeat(const char* token, int n) {
    std::string out;
    out.reserve(static_cast<std::size_t>(n) * 3);
    for (int i = 0; i < n; ++i)
        out += token;
    return out;
}

// Left-pads a cell's content out to `width` display columns, using
// displayWidth() instead of std::setw() so multi-byte cells (arrows,
// bars) still line up with the plain-ASCII border above/below them.
inline std::string padCell(const std::string& s, int width) {
    const int w = static_cast<int>(displayWidth(s));
    return w >= width ? s : s + std::string(static_cast<std::size_t>(width - w), ' ');
}

// Truncates a string to at most `width` visible columns, replacing
// the tail with a single-column ellipsis ("…") when it doesn't fit.
// padCell() only ever pads short content out to width -- it never
// shortens content that's already too wide, so a long benchmark or
// suite name would otherwise spill past its cell and shove every
// border/column to its right out of alignment. Walks codepoints
// (not bytes) so a multi-byte character never gets cut in half.
inline std::string truncateCell(const std::string& s, int width) {
    if (width <= 0)
        return "";
    if (static_cast<int>(displayWidth(s)) <= width)
        return s;
    if (width == 1)
        return "…";

    std::string out;
    int cols = 0;
    for (std::size_t i = 0; i < s.size();) {
        const unsigned char c = s[i];
        const std::size_t len = (c & 0x80) == 0      ? 1
                                : (c & 0xE0) == 0xC0 ? 2
                                : (c & 0xF0) == 0xE0 ? 3
                                                     : 4;
        if (cols >= width - 1)
            break;
        out += s.substr(i, len);
        ++cols;
        i += len;
    }
    return out + "…";
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

// Renders a mini bar chart for one value relative to the larger of the
// two values being compared -- shorter bar reads as faster, mirroring
// "smaller number is better". Unlike the benchmark tool, this tool
// doesn't use std::chrono (snapshot data loads as plain doubles), so
// this takes raw ns_per_op values directly rather than nanoseconds.
inline std::string bar(double ns, double maxNs) {
    if (maxNs <= 0.0)
        return repeat("░", BAR_W);

    int filled = static_cast<int>(std::lround(ns / maxNs * BAR_W));
    filled = std::clamp(filled, 0, BAR_W);

    return repeat("█", filled) + repeat("░", BAR_W - filled);
}

// Name of the operation whose row was printed last -- a method
// typically prints 3 rows (10K/100K/1M), then the next method switches
// to a new name. Tracked so printComparisonRow() can draw a real
// separator line the moment the name changes, instead of leaving a
// blank, borderless gap between one method's rows and the next. Reset
// by setHeader() at the start of each new table.
inline std::string lastRowGroup;

// Draws a mid-table separator line the moment the method name changes
// from the previous row -- called by printComparisonRow() before
// printing each row. A no-op for the first row of a table
// (lastRowGroup starts empty, reset by setHeader()) and for every row
// after the first that shares its predecessor's name (the 10K/100K/1M
// rows of the same method).
inline void drawGroupSeparatorIfNeeded(std::string_view name) {
    if (!lastRowGroup.empty() && lastRowGroup != name)
        drawBorder(ML, MM, MR, {TEST_W, ITER_W, VAL_W, VAL_W, DELTA_W});
    lastRowGroup = std::string(name);
}

// Prints the boxed comparison-table header for one suite.
inline void setHeader(std::string_view header) {
    lastRowGroup.clear();

    drawBorder(TL, TM, TR, {TEST_W, ITER_W, VAL_W, VAL_W, DELTA_W});

    // clang-format off
    std::cout << GRAY << V << RESET
              << BOLD << CYAN << padCell(" " + truncateCell(std::string(header), TEST_W - 1), TEST_W) << RESET
              << GRAY << V << RESET
              << BOLD << CYAN << padCell(" Iteration", ITER_W) << RESET
              << GRAY << V << RESET
              << BOLD << BLUE << padCell(" " + currentLabel, VAL_W) << RESET
              << GRAY << V << RESET
              << BOLD << MAGENTA << padCell(" " + baselineLabel, VAL_W) << RESET
              << GRAY << V << RESET
              << BOLD << CYAN << padCell(" Δ", DELTA_W) << RESET
              << GRAY << V << RESET << "\n";
    // clang-format on

    drawBorder(ML, MM, MR, {TEST_W, ITER_W, VAL_W, VAL_W, DELTA_W});

    markdown_buffer() += "\n## " + std::string(header) + "\n\n";
    markdown_buffer() +=
        "| Test | Iteration | " + currentLabel + " | " + baselineLabel + " | Δ |\n";
    markdown_buffer() += "|---|---|---|---|---|\n";
}

// Prints the closing border under a comparison table.
inline void endTable() {
    drawBorder(BL, BM, BR, {TEST_W, ITER_W, VAL_W, VAL_W, DELTA_W});
}

// Formats a magnitude that's already in its final display unit (e.g.
// a microsecond count as a plain double) to at most 5 significant
// digits: values under 1000 keep up to 2 decimal places (already
// <=5 sig figs), and values with more integer digits keep fewer
// decimals, dropping to 0 and rounding to the nearest 10/100/etc. once
// there are 5+ integer digits -- so formatDuration's per-unit number
// never grows past 5 significant digits, even for an oddly long-
// running "s"-scale benchmark. Shared with the framework's own
// helpers.h so both tools' tables round durations the same way.
inline std::string formatMagnitude(double value) {
    const double av = std::fabs(value);
    const int digits = (av < 1.0) ? 1 : static_cast<int>(std::floor(std::log10(av))) + 1;

    std::ostringstream out;
    if (digits >= 5) {
        const double factor = std::pow(10.0, digits - 5);
        const double rounded = std::round(value / factor) * factor;
        out << std::fixed << std::setprecision(0) << rounded;
    } else {
        const int decimals = std::min(2, 5 - digits);
        out << std::fixed << std::setprecision(decimals) << value;
    }
    return out.str();
}

// Formats a duration using the most appropriate time unit (ns/us/ms/s),
// so a fast row like "201.42 ns" and a slow row like "1.09 s" both stay
// short instead of a single fixed "ns" unit forcing huge digit counts
// like "271462.00 ns".
inline auto formatDuration(nanoseconds ns) {
    std::ostringstream out;

    if (ns < microseconds(1))
        out << ns.count() << " ns";
    else if (ns < milliseconds(1))
        out << formatMagnitude(duration<double, std::micro>(ns).count()) << " us";
    else if (ns < seconds(1))
        out << formatMagnitude(duration<double, std::milli>(ns).count()) << " ms";
    else
        out << formatMagnitude(duration<double>(ns).count()) << " s";

    return out.str();
}

// Overload for this tool's raw double nanoseconds (bNs/cNs, read
// straight from JSON snapshots) -- rounds to the nearest whole
// nanosecond and defers to the chrono overload above.
inline std::string formatDuration(double ns) {
    return formatDuration(nanoseconds(std::llround(ns)));
}

// Prints (and records, for export) one baseline-vs-current comparison
// row: name, iteration tier, both values each with a relative-size
// bar, and the % delta with an arrow and color -- red/▼ if the
// current run is meaningfully slower, green/▲ if meaningfully faster
// (bolded further past SIGNIFICANT_THRESHOLD), gray/— within
// NOISE_THRESHOLD. bNs is the baseline ns/op, cNs is the current run's
// ns/op -- printed under the columns set up by setHeader() (labeled
// via setLabels(), in "current, baseline" order).
inline void printComparisonRow(std::string_view suite, std::string_view name, std::size_t iter,
                               std::string_view iteration, double bNs, double cNs) {
    drawGroupSeparatorIfNeeded(name);

    const double pct = bNs == 0 ? 0.0 : (bNs - cNs) / cNs * 100.0;

    const bool noise = std::fabs(pct) < NOISE_THRESHOLD;
    const char* deltaColor = noise ? GRAY : (pct > 0.0) ? GREEN : RED;
    const char* arrow = noise ? "— " : (pct > 0.0) ? "▲ " : "▼ ";
    std::string boldDeltaColor =
        std::fabs(pct) >= SIGNIFICANT_THRESHOLD ? std::string(BOLD) + deltaColor : deltaColor;

    std::ostringstream deltaStream;
    deltaStream << std::showpos << std::fixed << std::setprecision(1) << pct << "%";

    std::ostringstream currentValStream;
    currentValStream << formatDuration(cNs);

    std::ostringstream baselineValStream;
    baselineValStream << formatDuration(bNs);

    const double maxNs = std::max(bNs, cNs);

    const std::string currentCell =
        " " + padCell(currentValStream.str(), DUR_W) + " " + bar(cNs, maxNs);
    const std::string baselineCell =
        " " + padCell(baselineValStream.str(), DUR_W) + " " + bar(bNs, maxNs);
    const std::string deltaCell = " " + std::string(arrow) + deltaStream.str();

    // clang-format off
    std::cout << GRAY << V << RESET
              << padCell(" " + truncateCell(std::string(name), TEST_W - 1), TEST_W)
              << GRAY << V << RESET
              << padCell(" " + std::string(iteration), ITER_W)
              << GRAY << V << RESET
              << BLUE << padCell(currentCell, VAL_W) << RESET
              << GRAY << V << RESET
              << MAGENTA << padCell(baselineCell, VAL_W) << RESET
              << GRAY << V << RESET
              << boldDeltaColor << padCell(deltaCell, DELTA_W) << RESET
              << GRAY << V << RESET << "\n";
    // clang-format on

    markdown_buffer() += "| " + std::string(name) + " | " + std::string(iteration) + " | " +
                         currentValStream.str() + " | " + baselineValStream.str() + " | " +
                         deltaStream.str() + " |\n";

    regression_results().push_back({std::string(suite), std::string(name), iter, bNs, cNs, pct});
}