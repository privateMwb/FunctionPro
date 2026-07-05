#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct ExampleSuite {
    int id;
    std::string name;
    void (*run)();
};

// Function-local static avoids the static initialization order fiasco:
// registrars in other translation units run before main() and need a
// guaranteed-initialized registry to register into.
inline std::vector<ExampleSuite>& example_registry()
{
    static std::vector<ExampleSuite> registry;
    return registry;
}

inline int& next_example_suite_id()
{
    static int id = 1;
    return id;
}

struct ExampleRegistrar {
    ExampleRegistrar(const char* file, void (*run)())
    {
        example_registry().push_back({
            next_example_suite_id()++,
            // Use the source file's stem as the suite name so each REGISTER_EXAMPLE_SUITE
            // macro invocation doesn't need to pass one explicitly.
            std::filesystem::path(file).stem().string(),
            run
        });
    }
};