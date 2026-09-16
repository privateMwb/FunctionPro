// clang-format off
#include "support/framework.h"  // printAllExampleSuite(), printExampleSuiteList(), printOneSuite()
                                // runSuites(), printUsage(), example_registry(), ExampleSuite
                                // prettify(), toLower()
// clang-format on

#include <vector> // std::vector (category-match collection)

int main(int argc, char* argv[]) {
    // No args: run every registered suite.
    if (argc == 1) {
        printAllExampleSuite();
        return 0;
    }

    std::string_view requested = argv[1];

    // "-h" / "--help": print usage and exit.
    if (requested == "-h" || requested == "--help") {
        printUsage();
        return 0;
    }

    // "list": print every registered suite, grouped by category, no run.
    if (requested == "list") {
        printExampleSuiteList();
        return 0;
    }

    // Otherwise: run whichever suite(s) match the requested name, id, or category.
    const std::string requestedLower = toLower(prettify(requested));
    std::vector<const ExampleSuite*> categoryMatches;

    for (const auto& suite : example_registry()) {
        const std::string nameLower = toLower(suite.name);
        const std::string idLower = toLower(suite.id);
        const std::string categoryLower = toLower(suite.category);

        // Exact suite match (by name or id): run just this one and exit.
        if (nameLower == requestedLower || idLower == requestedLower) {
            printOneSuite(suite);
            return 0;
        }

        // Category match: collect it, keep scanning for more.
        if (categoryLower == requestedLower)
            categoryMatches.push_back(&suite);
    }

    if (!categoryMatches.empty()) {
        runSuites(categoryMatches, 1);
        return 0;
    }

    std::cerr << "\nUnknown example suite: " << requested << "\n\n";
    return 1;
}