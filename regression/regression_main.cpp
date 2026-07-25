// clang-format off
#include "support/framework.h"  // setProjectLabels
                                // latestBaseline(), printList(), loadResults()
                                // printRegression(), exportJson(), exportMarkdown()
// clang-format on

// Loads baseline + current benchmark snapshots, prints the regression
// comparison, and exports the results as JSON and markdown reports.
int main(int argc, char* argv[]) {
    std::string baselineFile;
    std::string currentFile;

    setProjectLabels();

    try {
        if (argc == 1) {
            baselineFile = latestBaseline();
            currentFile = "benchmarks/results/benchmark_results.json";
        } else if (argc == 2) {
            if (std::string_view(argv[1]) == "list") {
                printList();
                return 0;
            }
            baselineFile = "benchmarks/baselines/" + std::string(argv[1]) + ".json";
            currentFile = "benchmarks/results/benchmark_results.json";
        } else if (argc == 3) {
            baselineFile = "benchmarks/baselines/" + std::string(argv[1]) + ".json";
            currentFile = "benchmarks/baselines/" + std::string(argv[2]) + ".json";
        } else {
            std::cerr << "Usage:\n"
                      << "  regression\n"
                      << "  regression <baseline>\n"
                      << "  regression <baseline> <current>\n";
            return 1;
        }

        auto baselineResults = loadResults(baselineFile);
        auto currentResults = loadResults(currentFile);

        printRegression(baselineResults, currentResults);

        exportJson("regression_results.json");
        exportMarkdown("regression_results.md");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}