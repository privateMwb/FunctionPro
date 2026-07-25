// clang-format off
#include "support/framework.h"  // printAllTestSuite(), printTestSuiteList(), printOneSuite()
                                // test_registry(), stats()
                                // prettify(), toLower()
// clang-format on

int main(int argc, char* argv[]) {
    // No args: run every registered suite.
    if (argc == 1) {
        printAllTestSuite();
        return 0;
    }

    std::string_view requested = argv[1];

    // "list": print every registered suite, grouped by category, no run.
    if (requested == "list") {
        printTestSuiteList();
        return 0;
    }

    // Otherwise: run whichever suite(s) match the requested name, id, or category.
    const std::string requestedLower = toLower(prettify(requested));
    bool foundCategory = false;

    for (const auto& suite : test_registry()) {
        const std::string nameLower = toLower(suite.name);
        const std::string idLower = toLower(suite.id);
        const std::string categoryLower = toLower(suite.category);

        // Exact suite match (by name or id): run just this one and exit.
        if (nameLower == requestedLower || idLower == requestedLower) {
            printOneSuite(suite);
            stats();
            std::cout << "\n";
            return 0;
        }

        // Category match: run every suite in it, keep scanning for more.
        if (categoryLower == requestedLower) {
            foundCategory = true;
            printOneSuite(suite);
        }
    }

    if (foundCategory) {
        stats();
        return 0;
    }

    std::cerr << "\nUnknown test suite: " << requested << "\n\n";
    return 1;
}