#include <common/framework.h>

#include <iomanip>

int main(int argc, char* argv[]) {
    // No args: run every registered suite.
    if (argc == 1) {
        for (const auto& suite : example_registry()) {
            std::cout << "\n";
            mainTitle(suite.name);
            suite.run();
        }
        std::cout << "\n";
        return 0;
    }

    std::string_view requested = argv[1];

    if (requested == "list") {
        std::cout << "\nAvailable example suites:\n";
        std::string category;
        for (const auto& suite : example_registry()) {
            if (category != suite.category)
              std::cout << '\n' << CYAN << prettify(suite.category) << RESET << '\n';
            std::cout << GREEN
                      << std::left << std::setw(6)
                      << ("[" + suite.id + "]")
                      << RESET
                      << std::setw(30)
                      << suite.name
                      << '\n';
            category = suite.category;
        }
        std::cout << "\n";
        return 0;
    }

    // Allow lookup by either name or numeric id.
    for (const auto& suite : example_registry()) {
        if (toLower(suite.name) == toLower(requested) ||
            toLower(suite.id) == toLower(requested)) {
            std::cout << "\n";
            mainTitle(suite.name);
            suite.run();
            std::cout << "\n";
            return 0;
        }
    }

    std::cerr << "\nUnknown example suite: " << requested << "\n\n";
    return 1;
}