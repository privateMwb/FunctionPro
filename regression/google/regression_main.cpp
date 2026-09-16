// clang-format off
#include "support/framework.h"  // setProjectLabels
                                // latestBaseline(), printList(), printMethods(), loadResults()
                                // printRegression(), printSummary(), exportJson(), exportMarkdown()
// clang-format on

namespace {

constexpr const char* kLiveResultsFile = "benchmarks/results/google_benchmark_results.json";

// Path to a Google Benchmark baseline snapshot's JSON file, given its
// version tag. The file inside the tag's folder is "g"-prefixed to
// distinguish it from the custom suite's snapshot sitting alongside
// it (e.g. benchmarks/baselines/v1.2.0/gv1.2.0.json).
std::string baselinePath(const Baseline& version) {
    return "benchmarks/baselines/" + version.name + "/g" + version.name + ".json";
}

// Column label for a comparison side: the version tag if it came from
// a tagged baseline folder, or "Current" for the live local run.
std::string labelFor(const std::string& file) {
    if (file == kLiveResultsFile)
        return "Current";
    return fs::path(file).parent_path().filename().string();
}

// Prints the CLI's usage summary: one aligned command/description
// table (optional trailing args collapsed with [<...>] bracket
// notation, so e.g. the plain-baseline and baseline+benchmark forms
// share a single row), followed by the <baseline>/<benchmark>
// placeholder legend and two usage notes (quoting, [LetterN]
// shortcuts). Command literals are cyan, subcommands/flags yellow,
// placeholders and descriptions gray -- unboxed, so it reads as a
// quick reference rather than a walkthrough.
void printUsage() {
    struct Segment {
        const char* color;
        std::string text;
    };
    struct Row {
        std::vector<Segment> command;
        std::string description;
    };

    const std::vector<Row> rows = {
        {{{CYAN, "google_regression"}}, "compare newest baseline vs current run"},
        {{{CYAN, "google_regression"}, {YELLOW, " list"}}, "list all baselines"},
        {{{CYAN, "google_regression"}, {YELLOW, " methods"}, {GRAY, " [<baseline>]"}},
         "list benchmarks (all, or one baseline)"},
        {{{CYAN, "google_regression"}, {GRAY, " <baseline>"}, {GRAY, " [<benchmark>]"}},
         "vs one baseline"},
        {{{CYAN, "google_regression"}, {GRAY, " <benchmark>"}}, "vs current run, one benchmark"},
        {{{CYAN, "google_regression"}, {GRAY, " <baseline> <baseline>"}}, "baseline vs baseline"},
        {{{CYAN, "google_regression"}, {GRAY, " <baseline> <baseline> <benchmark>"}},
         "baseline vs baseline, one benchmark"},
        {{{CYAN, "google_regression"}, {YELLOW, " -h | --help | help"}}, "this summary"},
    };

    std::size_t colWidth = 0;
    for (const auto& row : rows) {
        std::size_t len = 0;
        for (const auto& seg : row.command)
            len += displayWidth(seg.text);
        colWidth = std::max(colWidth, len);
    }
    colWidth += 3;

    std::cerr << BOLD << "Usage:" << RESET << "\n";

    for (const auto& row : rows) {
        std::size_t len = 0;
        std::cerr << "  ";
        for (const auto& seg : row.command) {
            std::cerr << seg.color << seg.text << RESET;
            len += displayWidth(seg.text);
        }
        std::cerr << std::string(colWidth - len, ' ') << DARK_GRAY << row.description << RESET
                  << "\n";
    }

    std::cerr << "\n"
              << GRAY << "<baseline>" << DARK_GRAY << " = version tag, e.g. v1.2.0    " << GRAY
              << "<benchmark>" << DARK_GRAY << " = benchmark name, e.g. BM_PushBack" << RESET
              << "\n\n"
              << DARK_GRAY
              << "quoting: a <benchmark> with spaces (rare, but Google Benchmark allows it)\n"
              << "doesn't need quotes -- every word after the tag(s) joins back into the name\n\n"
              << "shortcuts: <benchmark> can be a [LetterN] id from " << RESET << YELLOW
              << "methods" << RESET << DARK_GRAY << " (e.g. \"a1\")" << RESET << "\n";
}

} // namespace

// Loads baseline + current benchmark snapshots, prints the regression
// comparison (optionally restricted to a single benchmark), and
// exports the results as JSON and markdown reports.
int main(int argc, char* argv[]) {
    std::string baselineFile;
    std::string currentFile;

    setProjectLabels();

    try {
        // `list`/`methods` are exact, argument-less commands -- only
        // matched as the single argument, so a benchmark that
        // happened to share one of these names wouldn't be shadowed
        // by a trailing name filter.
        if (argc == 2) {
            std::string_view arg1 = argv[1];

            if (arg1 == "list") {
                printList();
                return 0;
            }
            if (arg1 == "methods") {
                printMethods();
                return 0;
            }
            if (arg1 == "-h" || arg1 == "--help" || arg1 == "help") {
                printUsage();
                return 0;
            }
        }

        // `methods <baseline>` is its own exact, two-argument form --
        // checked before the general version-arg parsing below so
        // "methods" (which isn't itself a version tag) doesn't fall
        // through and get treated as a benchmark filter against the
        // latest baseline.
        if (argc == 3 && std::string_view(argv[1]) == "methods") {
            printMethods(argv[2]);
            return 0;
        }

        // Up to two leading arguments may be version tags (current,
        // then baseline). Whatever's left after that -- possibly
        // several words -- is joined back with spaces into the
        // benchmark-name filter, so a name containing spaces works
        // without the caller needing to quote it.
        Baseline v1, v2;
        int versionCount = 0;

        if (argc >= 2 && tryParseVersionArg(argv[1], v1)) {
            versionCount = 1;
            if (argc >= 3 && tryParseVersionArg(argv[2], v2))
                versionCount = 2;
        }

        std::string methodFilter;
        for (int i = 1 + versionCount; i < argc; ++i) {
            if (!methodFilter.empty())
                methodFilter += " ";
            methodFilter += argv[i];
        }

        if (versionCount == 2) {
            // google_regression <baseline> <baseline> [benchmark]
            currentFile = baselinePath(v1);
            baselineFile = baselinePath(v2);
        } else if (versionCount == 1) {
            // google_regression <baseline> [benchmark]
            baselineFile = baselinePath(v1);
            currentFile = kLiveResultsFile;
        } else {
            // google_regression [benchmark]: latest baseline vs. the
            // live local run -- with or without a name filter.
            baselineFile = latestBaseline();
            currentFile = kLiveResultsFile;
        }

        std::string currentColumnLabel = labelFor(currentFile);
        std::string baselineColumnLabel = labelFor(baselineFile);
        setLabels(currentColumnLabel, baselineColumnLabel);

        // A benchmark filter matching the `methods` command's
        // [LetterN] shape (e.g. "a1") is resolved against the
        // baseline's own letter-grouped name list before filtering,
        // so `google_regression a1` works the same way `tests A1`
        // runs a suite by id.
        if (!methodFilter.empty() && looksLikeMethodId(methodFilter))
            methodFilter = resolveMethodId(methodFilter, baselineFile);

        auto baselineResults = loadResults(baselineFile);
        auto currentResults = loadResults(currentFile);

        if (!methodFilter.empty()) {
            applyMethodFilter(baselineResults, currentResults, methodFilter, currentColumnLabel,
                              baselineColumnLabel);
        }

        printRegression(baselineResults, currentResults);
        printSummary();

        exportJson("google_regressions_results.json");
        exportMarkdown("google_regressions_results.md");
    } catch (const std::exception& e) {
        std::cerr << BOLD << RED << "Error: " << RESET << e.what() << '\n';
        return 1;
    }

    return 0;
}
