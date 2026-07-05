#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct TestSuite {
    int id;
    std::string name;
    void (*run)();
};

// Function-local static avoids the static initialization order fiasco:
// registrars in other translation units run before main() and need a
// guaranteed-initialized registry to register into.
inline std::vector<TestSuite>& test_registry()
{
    static std::vector<TestSuite> registry;
    return registry;
}

inline int& next_test_suite_id()
{
    static int id = 1;
    return id;
}

struct TestRegistrar {
    TestRegistrar(const char* file, void (*run)())
    {
        test_registry().push_back({
            next_test_suite_id()++,
            // Use the source file's stem as the suite name so each TEST_SUITE
            // macro invocation doesn't need to pass one explicitly.
            std::filesystem::path(file).stem().string(),
            run
        });
    }
};