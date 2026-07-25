#pragma once

// clang-format off
#include "helpers.h"      // prettify()

#include <cctype>         // std::toupper
#include <filesystem>     // std::filesystem::path
#include <string>         // std::string
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector
// clang-format on

// One registered example suite: an id ("E1"), the category it belongs to
// (its containing directory name), a display name, and the function that
// runs it. Populated by ExampleRegistrar's constructor at static-init time.
struct ExampleSuite {
    std::string id;
    std::string category;
    std::string name;
    void (*run)();
};

// Global registry of every example suite discovered via
// REGISTER_EXAMPLE_SUITE(). Order reflects static-initialization order.
inline std::vector<ExampleSuite>& example_registry() {
    static std::vector<ExampleSuite> registry;
    return registry;
}

// Tracks how many suites have been registered per category so far, so
// ExampleRegistrar can assign each suite a sequential id within its
// category ("E1", "E2", ...).
inline std::unordered_map<std::string, int>& category_counters() {
    static std::unordered_map<std::string, int> counters;
    return counters;
}

// Registers one example suite at static-init time. Constructing an
// ExampleRegistrar (via REGISTER_EXAMPLE_SUITE()) derives the suite's
// category from its containing directory, assigns it a sequential id
// within that category, and appends it to example_registry().
struct ExampleRegistrar {
    ExampleRegistrar(const char* file, void (*run)()) {
        auto path = std::filesystem::path(file);

        std::string category = path.parent_path().filename().string();

        char prefix = std::toupper(static_cast<unsigned char>(category.front()));

        int number = ++category_counters()[category];

        example_registry().push_back({std::string(1, prefix) + std::to_string(number), category,
                                      prettify(path.stem().string()), run});
    }
};