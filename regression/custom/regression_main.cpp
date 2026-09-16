// clang-format off
#include "support/framework.h"  // setProjectLabels
                                // latestBaseline(), printList(), printMethods(), loadResults()
                                // printRegression(), printSummary(), exportJson(), exportMarkdown()
// clang-format on

namespace {

constexpr const char* kLiveResultsFile = "benchmarks/results/benchmark_results.json";

// Path to a baseline snapshot's JSON file, given its version tag.
std::string baselinePath(const Baseline& version) {
    return "benchmarks/baselines/" + version.name + "/" + version.name + ".json";
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
// notation, so e.g. the plain-baseline and baseline+method forms
// share a single row), followed by the <baseline>/<method>
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
        {{{CYAN, "regression"}}, "compare newest baseline vs current run"},
        {{{CYAN, "regression"}, {YELLOW, " list"}}, "list all baselines"},
        {{{CYAN, "regression"}, {YELLOW, " methods"}, {GRAY, " [<baseline>]"}},
         "list methods (all, or one baseline)"},
        {{{CYAN, "regression"}, {GRAY, " <baseline>"}, {GRAY, " [<method>]"}}, "vs one baseline"},
        {{{CYAN, "regression"}, {GRAY, " <method>"}}, "vs current run, one method"},
        {{{CYAN, "regression"}, {GRAY, " <baseline> <baseline>"}}, "baseline vs baseline"},
        {{{CYAN, "regression"}, {GRAY, " <baseline> <baseline> <method>"}},
         "baseline vs baseline, one method"},
        {{{CYAN, "regression"}, {YELLOW, " -h | --help | help"}}, "this summary"},
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
              << "<method>" << DARK_GRAY << " = benchmark name, e.g. push_back" << RESET << "\n\n"
              << DARK_GRAY
              << "quoting: a <method> with spaces/commas doesn't need quotes -- every word after\n"
              << "the tag(s) joins back into the name\n\n"
              << "shortcuts: <method> can be a [LetterN] id from " << RESET << YELLOW << "methods"
              << RESET << DARK_GRAY << " (e.g. \"u1\")" << RESET << "\n";
}

} // namespace

// Loads baseline + current benchmark snapshots, prints the regression
// comparison (optionally restricted to a single benchmark method),
// and exports the results as JSON and markdown reports.
int main(int argc, char* argv[]) {
    std::string baselineFile;
    std::string currentFile;

    setProjectLabels();

    try {
        // `list`/`methods` are exact, argument-less commands -- only
        // matched as the single argument, so a method that happened
        // to share one of these names wouldn't be shadowed by a
        // trailing method filter.
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
        // through and get treated as a method filter against the
        // latest baseline.
        if (argc == 3 && std::string_view(argv[1]) == "methods") {
            printMethods(argv[2]);
            return 0;
        }

        // Up to two leading arguments may be version tags (current,
        // then baseline). Whatever's left after that -- possibly
        // several words -- is joined back with spaces into the
        // method filter, so a method name containing spaces/commas
        // (e.g. "push_back, 0 listeners") works without the caller
        // needing to quote it.
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
            // regression <current> <baseline> [method]
            currentFile = baselinePath(v1);
            baselineFile = baselinePath(v2);
        } else if (versionCount == 1) {
            // regression <baseline> [method]
            baselineFile = baselinePath(v1);
            currentFile = kLiveResultsFile;
        } else {
            // regression [method]: latest baseline vs. the live local
            // run -- with or without a method filter.
            baselineFile = latestBaseline();
            currentFile = kLiveResultsFile;
        }

        std::string currentColumnLabel = labelFor(currentFile);
        std::string baselineColumnLabel = labelFor(baselineFile);
        setLabels(currentColumnLabel, baselineColumnLabel);

        // A method filter matching the `methods` command's [LetterN]
        // shape (e.g. "u1") is resolved against the baseline's own
        // letter-grouped method list before filtering, so `regression
        // u1` works the same way `tests U1` runs a suite by id.
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

        exportJson("regression_results.json");
        exportMarkdown("regression_results.md");
    } catch (const std::exception& e) {
        std::cerr << BOLD << RED << "Error: " << RESET << e.what() << '\n';
        return 1;
    }

    return 0;
}
