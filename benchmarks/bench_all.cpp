#include <common/framework.h>

int main(int argc, char* argv[]) {
    // No args: run every registered suite.
    if (argc == 1) {
        for (const auto& suite : bench_registry()) {
            std::cout << "\n";
            setHeader(suite.name);
            suite.run();
            borderLine();
        }
        std::cout << "\n";
        return 0;
    }

    std::string_view requested = argv[1];
    int requested_id = -1;

    try {
      requested_id = std::stoi(std::string(requested));
    } catch (...) {
      // Not a number.
    }

    if (requested == "list") {
        std::cout << "\nAvailable bench suites:\n";
        for (const auto& suite : bench_registry())
            std::cout << suite.id << ".  " 
                      << prettify(suite.name) << '\n';
        std::cout << "\n";
        return 0;
    }

    // Allow lookup by either name or numeric id.
    for (const auto& suite : bench_registry()) {
        if (suite.name == requested ||
            suite.id == requested_id) {
            std::cout << "\n";
            setHeader(suite.name);
            suite.run();
            borderLine();
            std::cout << "\n";
            return 0;
        }
    }

    std::cerr << "\nUnknown bench suite: " << requested << "\n\n";
    return 1;
}