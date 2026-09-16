#pragma once

// clang-format off
#include <algorithm>              // std::transform, std::clamp
#include <chrono>                 // nanoseconds, duration<>, used throughout formatDuration
#include <cmath>                  // std::lround
#include <iomanip>                // std::setw, std::setprecision, std::fixed, std::left
#include <iostream>               // std::cout
#include <sstream>                // std::ostringstream
#include <string>                 // std::string
#include <string_view>            // std::string_view — used in every function signature below
// clang-format on

using namespace std::chrono;

// ── Constants ───────────────────────────────────────────────────────

// ANSI terminal color/style codes.
// clang-format off
inline constexpr const char* RESET   = "\033[0m";
inline constexpr const char* BOLD    = "\033[1m";
inline constexpr const char* DIM     = "\033[2m";
inline constexpr const char* GREEN   = "\033[92m";
inline constexpr const char* YELLOW  = "\033[93m";
inline constexpr const char* RED     = "\033[91m";
inline constexpr const char* CYAN    = "\033[96m";
inline constexpr const char* GRAY    = "\033[90m";
inline constexpr const char* BLUE    = "\033[94m";   // the "custom" implementation's color
inline constexpr const char* MAGENTA = "\033[95m";   // the "standard" implementation's color

// Benchmark iteration presets.
inline constexpr std::size_t SMALL  = 10'000;
inline constexpr std::size_t MEDIUM = 100'000;
inline constexpr std::size_t LARGE  = 1'000'000;

// Column widths shared by the box-drawn tables and their borders — a
// border segment's dash count must always match the setw() of the
// cell above it, so both are defined from these constants.
inline constexpr int TEST_W     = 26;
inline constexpr int ITER_W     = 11;
inline constexpr int VAL_W      = 24;   // duration + bar, comparison table
inline constexpr int DELTA_W    = 14;   // fits large deltas like "▲ +93904.5%" without overflow
inline constexpr int BAR_W      = 10;   // characters in the mini bar chart
inline constexpr int SOLO_VAL_W = 20;   // duration only, solo table's one value column
inline constexpr int DUR_W      = 10;   // fixed width reserved for the duration text
                                        // before a bar, so bars in the same column
                                        // all start at the same x — otherwise
                                        // "9.99 ms" vs "114.77 ms" vs "1.09 s" push
                                        // the bar to a different offset each row.
// clang-format on

// ── Run state ───────────────────────────────────────────────────────

// Display labels for the two sides of a comparison, and the name of
// the suite currently running. Set via setCustom()/setStandard()/
// setSuite() before benchmarks run, then read by the print* functions.
inline std::string custom;
inline std::string standard;
inline std::string suiteName;

// Name of the operation (e.g. "Copy Construct") whose row was printed
// last — a BENCH()/BENCH_SOLO() call prints 3 rows (10K/100K/1M) under
// the same name, then the next call switches to a new name. Tracked so
// printComparisonRow()/printSoloRow() can draw a real separator line
// the moment the name changes, instead of leaving a blank, borderless
// gap between one operation's rows and the next. Reset whenever a new
// table is opened.
inline std::string lastRowGroup;

// Whether a table is currently open (header drawn, footer not yet
// printed) and, if so, whether it's the 3-column solo layout or the
// 5-column comparison layout. Tables are opened lazily by the first
// printComparisonRow()/printSoloRow() call after beginSuite() rather
// than eagerly by the caller, since a suite's mix of BENCH()/
// BENCH_SOLO() calls isn't known until it actually runs — see
// ensureTable() below.
inline bool tableOpen = false;
inline bool tableIsSolo = false;

// Accumulates a markdown-formatted transcript of the run, built up by
// setHeader()/printComparisonRow() alongside their normal stdout printing.
// Kept here rather than reconstructed later from benchmark_results(),
// since that list is flat, unpaired, and ungrouped by suite.
inline std::string& markdown_buffer() {
    static std::string buffer;
    return buffer;
}

// Tallies how many comparison rows the custom implementation won,
// lost, or tied against the reference — accumulated by
// printComparisonRow() as it runs, read by printSummary() once at the
// end of a full run. Solo benchmarks (no reference to compare against)
// don't contribute.
struct ResultTally {
    int customWins = 0;
    int standardWins = 0;
    int ties = 0;

    int total() const {
        return customWins + standardWins + ties;
    }
};
inline ResultTally& resultTally() {
    static ResultTally tally;
    return tally;
}

// Sets the display label shown for each side of a comparison row.
inline void setCustom(std::string name) {
    custom = name;
}
inline void setStandard(std::string name) {
    standard = name;
}

// Sets the current benchmark suite name (shown in table headers, and
// recorded into benchmark_results() so exports can group by suite).
inline void setSuite(const std::string name) {
    suiteName = name;
}

// Returns the current benchmark suite name set by setSuite().
inline const std::string getSuite() {
    return suiteName;
}

// Called once before a suite runs: records its name and clears the
// per-suite table state, so the first BENCH()/BENCH_SOLO() call inside
// suite.run() opens a fresh table. See ensureTable() further down.
inline void beginSuite(const std::string& name) {
    setSuite(name);
    tableOpen = false;
    lastRowGroup.clear();
} // ── String utilities ────────────────────────────────────────────────

// Formats a duration using the most appropriate time unit.
inline auto formatDuration(nanoseconds ns) {
    std::ostringstream out;

    if (ns < microseconds(1))
        out << ns.count() << " ns";
    else if (ns < milliseconds(1))
        out << std::fixed << std::setprecision(2) << duration<double, std::micro>(ns).count()
            << " us";
    else if (ns < seconds(1))
        out << std::fixed << std::setprecision(2) << duration<double, std::milli>(ns).count()
            << " ms";
    else
        out << std::fixed << std::setprecision(2) << duration<double>(ns).count() << " s";

    return out.str();
}

// Converts a snake_case function name to Title Case.
inline std::string prettify(std::string_view text) {
    std::string result{text};
    bool firstLetter = true;

    for (char& c : result) {
        if (firstLetter) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            firstLetter = false;
        } else if (c == '_' || c == ' ') {
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

// ── Optimization guards ─────────────────────────────────────────────

// Prevents the compiler from optimizing away benchmarked values.
template <typename T> inline void doNotOptimize(const T& value) {
#if defined(__GNUC__) || defined(__clang__)
    // "g" constraint forces the value into a register/memory and the
    // "memory" clobber stops the compiler from reordering or eliding
    // the computation that produced it.
    asm volatile("" : : "g"(value) : "memory");
#else
    // Weaker fallback for compilers without inline asm support: taking
    // a volatile pointer discourages (but doesn't guarantee against)
    // the optimizer dropping the computation.
    volatile const T* p = &value;
    (void)p;
#endif
}

// Prevents the compiler from optimizing away benchmarked code.
inline void doNotOptimize() {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" ::: "memory");
#endif
}

// ── Output / table printing ─────────────────────────────────────────

// Box-drawing pieces. UTF-8 multi-byte characters, so borders are built
// by repeating these tokens rather than via std::string(n, ch).
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

// Counts *display columns*, not bytes — std::setw counts raw bytes, so
// any multi-byte UTF-8 character (the bar's █/░, the Δ symbol, arrows,
// the em dash) makes it under-pad and drift the column out from under
// the border above/below it. Every UTF-8 character used in this file is
// single-width in a monospace terminal, so it's enough to just skip
// continuation bytes (0x80-0xBF) when counting.
inline int visualWidth(const std::string& s) {
    int width = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80)
            ++width;
    return width;
}

// Left-pads a cell's content out to `width` display columns, using
// visualWidth() instead of std::setw() so multi-byte cells still line
// up with the plain-ASCII border above/below them.
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

// Renders a mini bar chart for one duration relative to the larger of
// the two durations being compared — shorter bar reads as faster,
// mirroring "smaller number is better".
inline std::string bar(nanoseconds ns, nanoseconds maxNs) {
    if (maxNs.count() <= 0)
        return repeat("░", BAR_W);

    int filled = static_cast<int>(
        std::lround(static_cast<double>(ns.count()) / static_cast<double>(maxNs.count()) * BAR_W));
    filled = std::clamp(filled, 0, BAR_W);

    return repeat("█", filled) + repeat("░", BAR_W - filled);
}

// Legacy plain separator, kept for anything printed outside the boxed
// tables (e.g. the suite list).
inline void borderLine() {
    std::cout << GRAY << repeat(H, 90) << RESET << "\n";
}

// Prints the boxed comparison-table header (5 columns: Test, Iteration,
// the two implementations, and Δ). Called lazily by ensureTable() —
// see below — not directly by suite-driving code, since a suite's
// table type isn't known until its first row comes in.
inline void setHeader(std::string_view header) {
    drawBorder(TL, TM, TR, {TEST_W, ITER_W, VAL_W, VAL_W, DELTA_W});

    // clang-format off
    std::cout << GRAY << V << RESET
              << BOLD << CYAN << padCell(" " + prettify(header), TEST_W) << RESET
              << GRAY << V << RESET
              << BOLD << CYAN << padCell(" Iteration", ITER_W) << RESET
              << GRAY << V << RESET
              << BOLD << BLUE << padCell(" " + custom, VAL_W) << RESET
              << GRAY << V << RESET
              << BOLD << MAGENTA << padCell(" " + standard, VAL_W) << RESET
              << GRAY << V << RESET
              << BOLD << CYAN << padCell(" Δ", DELTA_W) << RESET
              << GRAY << V << RESET << "\n";
    // clang-format on

    drawBorder(ML, MM, MR, {TEST_W, ITER_W, VAL_W, VAL_W, DELTA_W});

    markdown_buffer() += "\n## " + prettify(header) + "\n\n";
    markdown_buffer() += "| Test | Iteration | " + custom + " | " + standard + " | Δ |\n";
    markdown_buffer() += "|---|---|---|---|---|\n";
}

// Prints the closing border under a comparison table.
inline void endTable() {
    drawBorder(BL, BM, BR, {TEST_W, ITER_W, VAL_W, VAL_W, DELTA_W});
}

// Prints the boxed solo-table header: just Test, Iteration, and the
// one implementation being measured — no reference to diff against,
// so no second value column, bar, or Δ column at all.
inline void setSoloHeader(std::string_view header) {
    drawBorder(TL, TM, TR, {TEST_W, ITER_W, SOLO_VAL_W});

    // clang-format off
    std::cout << GRAY << V << RESET
              << BOLD << CYAN << padCell(" " + prettify(header), TEST_W) << RESET
              << GRAY << V << RESET
              << BOLD << CYAN << padCell(" Iteration", ITER_W) << RESET
              << GRAY << V << RESET
              << BOLD << BLUE << padCell(" " + custom, SOLO_VAL_W) << RESET
              << GRAY << V << RESET << "\n";
    // clang-format on

    drawBorder(ML, MM, MR, {TEST_W, ITER_W, SOLO_VAL_W});

    markdown_buffer() += "\n## " + prettify(header) + "\n\n";
    markdown_buffer() += "| Test | Iteration | " + custom + " |\n";
    markdown_buffer() += "|---|---|---|\n";
}

// Prints the closing border under a solo table.
inline void endSoloTable() {
    drawBorder(BL, BM, BR, {TEST_W, ITER_W, SOLO_VAL_W});
}

// Opens a table of the requested kind if none is open yet, or if the
// one that's open is the wrong kind — closing it first so a suite that
// mixes BENCH() and BENCH_SOLO() calls gets two clean, separate tables
// back to back rather than one another can't cleanly represent.
// Called at the top of printComparisonRow()/printSoloRow(); suite
// code never calls setHeader()/setSoloHeader() directly.
inline void closeTable(); // fwd decl — defined just below, mutually used by ensureTable()
inline void ensureTable(bool solo) {
    if (tableOpen && tableIsSolo != solo) {
        closeTable();
        std::cout << "\n";
    }

    if (!tableOpen) {
        if (solo)
            setSoloHeader(suiteName);
        else
            setHeader(suiteName);
        tableOpen = true;
        tableIsSolo = solo;
        lastRowGroup.clear();
    }
}

// Prints the closing border for whichever table kind is currently
// open, then marks no table as open. Called by ensureTable() when
// switching kinds mid-suite, and by beginSuite()'s caller (framework.h)
// once a suite finishes, so its final table gets closed. A no-op if no
// table is open — e.g. a suite with zero benchmarks.
inline void closeTable() {
    if (!tableOpen)
        return;
    if (tableIsSolo)
        endSoloTable();
    else
        endTable();
    tableOpen = false;
}

// Draws a mid-table separator line the moment the operation name
// changes from the previous row — called by printComparisonRow()/
// printSoloRow() before printing each row. A no-op for the first row
// of a table (lastRowGroup starts empty, reset by ensureTable() when
// it opens a table) and for every row after the first that shares its
// predecessor's name (the 10K/100K/1M rows of the same operation).
inline void drawGroupSeparatorIfNeeded(std::string_view name) {
    if (!lastRowGroup.empty() && lastRowGroup != name) {
        if (tableIsSolo)
            drawBorder(ML, MM, MR, {TEST_W, ITER_W, SOLO_VAL_W});
        else
            drawBorder(ML, MM, MR, {TEST_W, ITER_W, VAL_W, VAL_W, DELTA_W});
    }
    lastRowGroup = std::string(name);
}

// Prints one solo row: name, iteration tier, single duration — under
// its own 3-column table (opened automatically on the first call after
// beginSuite()), entirely separate from the comparison table's layout.
inline void printSoloRow(std::string_view name, std::string_view iteration, nanoseconds ns) {
    ensureTable(/*solo=*/true);
    drawGroupSeparatorIfNeeded(name);

    // clang-format off
    std::cout << GRAY << V << RESET
              << padCell(" " + prettify(name), TEST_W)
              << GRAY << V << RESET
              << padCell(" " + std::string(iteration), ITER_W)
              << GRAY << V << RESET
              << BLUE << padCell(" " + formatDuration(ns), SOLO_VAL_W) << RESET
              << GRAY << V << RESET << "\n";

    markdown_buffer() += "| " + prettify(name)
                      + " | " + std::string(iteration)
                      + " | " + formatDuration(ns)
                      + " |\n";
    // clang-format on
}

// Prints one merged comparison row: name, iteration tier, both durations
// each with a relative-size bar, and the % delta (custom vs reference)
// with an arrow and color — red/▼ if the custom implementation is
// meaningfully slower, green/▲ if meaningfully faster, gray/— within
// the noise threshold. Also tallies the result into resultTally() for
// the end-of-run summary.
inline void printComparisonRow(std::string_view name, std::string_view iteration,
                               nanoseconds customNs, nanoseconds stdNs) {
    ensureTable(/*solo=*/false);
    drawGroupSeparatorIfNeeded(name);

    const double pct =
        stdNs.count() == 0
            ? 0.0
            : (static_cast<double>(stdNs.count()) - static_cast<double>(customNs.count())) /
                  static_cast<double>(customNs.count()) * 100.0;

    // +-0% treated as measurement noise, not a real signal — see the
    // Termux big.LITTLE / thermal-throttling discussion.
    const char* deltaColor = (pct > 0.0) ? GREEN : (pct < 0.0) ? RED : GRAY;
    const char* arrow = (pct > 0.0) ? "▲ " : (pct < 0.0) ? "▼ " : "— ";

    if (pct > 0.0)
        ++resultTally().customWins;
    else if (pct < 0.0)
        ++resultTally().standardWins;
    else
        ++resultTally().ties;

    const nanoseconds maxNs = std::max(customNs, stdNs);

    std::ostringstream deltaStream;
    deltaStream << std::showpos << std::fixed << std::setprecision(1) << pct << "%";

    const std::string customCell =
        " " + padCell(formatDuration(customNs), DUR_W) + " " + bar(customNs, maxNs);
    const std::string stdCell =
        " " + padCell(formatDuration(stdNs), DUR_W) + " " + bar(stdNs, maxNs);
    const std::string deltaCell = " " + std::string(arrow) + deltaStream.str();

    // clang-format off
    std::cout << GRAY << V << RESET
              << padCell(" " + prettify(name), TEST_W)
              << GRAY << V << RESET
              << padCell(" " + std::string(iteration), ITER_W)
              << GRAY << V << RESET
              << BLUE << padCell(customCell, VAL_W) << RESET
              << GRAY << V << RESET
              << MAGENTA << padCell(stdCell, VAL_W) << RESET
              << GRAY << V << RESET
              << BOLD << deltaColor << padCell(deltaCell, DELTA_W) << RESET
              << GRAY << V << RESET << "\n";

    markdown_buffer() += "| " + prettify(name)
                      + " | " + std::string(iteration)
                      + " | " + formatDuration(customNs)
                      + " | " + formatDuration(stdNs)
                      + " | " + deltaStream.str()
                      + " |\n";
    // clang-format on
}

// Prints a boxed summary of every comparison row seen so far this run:
// how many the custom implementation won, lost, or tied, with a share
// bar. Called once, after every suite in the run has finished. No-op
// (prints nothing) if no comparison rows were recorded — e.g. a
// solo-only run.
inline void printSummary() {
    const ResultTally& t = resultTally();
    if (t.total() == 0)
        return;

    const int labelW = 20;
    const int valueW = 26;

    std::cout << "\n";
    drawBorder(TL, TM, TR, {labelW, valueW});

    std::cout << GRAY << V << RESET << BOLD << CYAN << padCell(" Summary", labelW) << RESET << GRAY
              << V << RESET << padCell(" " + std::to_string(t.total()) + " comparisons", valueW)
              << GRAY << V << RESET << "\n";

    drawBorder(ML, MM, MR, {labelW, valueW});

    auto row = [&](const char* color, const std::string& label, int count) {
        const double share = static_cast<double>(count) / static_cast<double>(t.total()) * 100.0;

        std::ostringstream countPct;
        countPct << count << " (" << std::fixed << std::setprecision(1) << share << "%)";

        const std::string val = " " + padCell(countPct.str(), 12) + " " +
                                bar(nanoseconds(static_cast<long long>(share)), nanoseconds(100));

        std::cout << GRAY << V << RESET << color << padCell(" " + label, labelW) << RESET << GRAY
                  << V << RESET << color << padCell(val, valueW) << RESET << GRAY << V << RESET
                  << "\n";

        markdown_buffer() += "| " + label + " | " + std::to_string(count) + " (" +
                             std::to_string(static_cast<int>(share)) + "%) |\n";
    };

    markdown_buffer() += "\n## Summary\n\n";
    markdown_buffer() += "| Result | Count |\n|---|---|\n";

    row(BLUE, custom + " faster", t.customWins);
    row(MAGENTA, standard + " faster", t.standardWins);
    row(GRAY, "Tie", t.ties);

    drawBorder(BL, BM, BR, {labelW, valueW});
    std::cout << "\n";
}