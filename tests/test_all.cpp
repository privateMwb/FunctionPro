#include <common/framework.h>

#include <iomanip>

int main(int argc, char* argv[]) {
    // No args: run every registered suite.
    if (argc == 1) {
        for (const auto& suite : test_registry()) {
            std::cout << "\n";
            setTitle(suite.name);
            suite.run();
        }
        std::cout << "\n";
        stats();
        std::cout << "\n";
        return 0;
    }

    std::string_view requested = argv[1];

    if (requested == "list") {
        std::cout << "\nAvailable test suites:\n";
        std::string category;
        for (const auto& suite : test_registry()) {
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
    for (const auto& suite : test_registry()) {
        if (toLower(suite.name) == toLower(requested) ||
            toLower(suite.id) == toLower(requested)) {
            std::cout << "\n";
            setTitle(suite.name);
            suite.run();
            std::cout << "\n";
            stats();
            std::cout << "\n";
            return 0;
        }
    }

    std::cerr << "\nUnknown test suite: " << requested << "\n\n";
    return 1;
}