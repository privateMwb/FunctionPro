#pragma once

// clang-format off
#include "helpers.h"      // prettify()

#include <cctype>         // std::toupper
#include <filesystem>     // std::filesystem::path
#include <string>         // std::string
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector
// clang-format on

// One registered test suite: an id ("T1"), the category it belongs to (its
// containing directory name), a display name, and the function that runs
// it. Populated by TestRegistrar's constructor at static-init time.
struct TestSuite {
    std::string id;
    std::string category;
    std::string name;
    void (*run)();
};

// Global registry of every test suite discovered via REGISTER_TEST_SUITE().
// Order reflects static-initialization order.
inline std::vector<TestSuite>& test_registry() {
    static std::vector<TestSuite> registry;
    return registry;
}

// Tracks how many suites have been registered per category so far, so
// TestRegistrar can assign each suite a sequential id within its category
// ("T1", "T2", ...).
inline std::unordered_map<std::string, int>& category_counters() {
    static std::unordered_map<std::string, int> counters;
    return counters;
}

// Registers one test suite at static-init time. Constructing a
// TestRegistrar (via REGISTER_TEST_SUITE()) derives the suite's category
// from its containing directory, assigns it a sequential id within that
// category, and appends it to test_registry().
struct TestRegistrar {
    TestRegistrar(const char* file, void (*run)()) {
        auto path = std::filesystem::path(file);

        std::string category = path.parent_path().filename().string();

        char prefix = std::toupper(static_cast<unsigned char>(category.front()));

        int number = ++category_counters()[category];

        test_registry().push_back({std::string(1, prefix) + std::to_string(number), category,
                                   prettify(path.stem().string()), run});
    }
};