// clang-format off
#include "support/framework.h"  // setProjectLabels()
                                // printAllBenchSuite(), printBenchSuiteList(), printOneSuite()
                                // bench_registry(), prettify(), toLower()
                                // exportJson(), exportMarkdown()

#include <iomanip>              // std::setw
// clang-format on

int main(int argc, char* argv[]) {
    setProjectLabels("FunctionPro", "std::function");

    // No args: run every registered suite.
    if (argc == 1) {
        printAllBenchSuite();
        return 0;
    }

    std::string_view requested = argv[1];

    if (requested == "list") {
        printBenchSuiteList();
        return 0;
    }

    // Otherwise: run whichever suite(s) match the requested name, id, or category.
    const std::string requestedLower = toLower(prettify(requested));
    bool foundCategory = false;

    for (const auto& suite : bench_registry()) {
        const std::string nameLower = toLower(suite.name);
        const std::string idLower = toLower(suite.id);
        const std::string categoryLower = toLower(suite.category);

        // Exact suite match (by name or id): run just this one and exit.
        if (nameLower == requestedLower || idLower == requestedLower) {
            printOneSuite(suite);

            exportJson("benchmark_results.json");
            exportMarkdown("benchmark_results.md");
            return 0;
        }

        // Category match: run every suite in it, keep scanning for more.
        if (categoryLower == requestedLower) {
            foundCategory = true;

            printOneSuite(suite);
        }
    }

    if (foundCategory) {
        exportJson("benchmark_results.json");
        exportMarkdown("benchmark_results.md");
        return 0;
    }

    std::cerr << "\nUnknown bench suite: " << requested << "\n\n";
    return 1;
}