#pragma once

// clang-format off
#include "helpers.h"      // prettify()

#include <cctype>         // std::toupper
#include <cstdio>         // std::fprintf
#include <exception>      // std::terminate, std::exception
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
    // Every operation below (string/path construction, the map lookup,
    // the vector push_back) can allocate and therefore throw bad_alloc.
    // That's fine in itself, but this constructor runs as part of a
    // static-storage-duration object's initialization (see
    // REGISTER_TEST_SUITE()), which happens before main() -- an
    // exception escaping from here can't be caught anywhere and the
    // program terminates uncontrolled either way. Marking this noexcept
    // and handling failure explicitly turns that implicit, opaque
    // terminate into a deliberate, diagnosable one instead. Realistically
    // this only fires under allocation failure, at which point running
    // the test suite at all would be meaningless.
    TestRegistrar(const char* file, void (*run)()) noexcept {
        try {
            auto path = std::filesystem::path(file);

            std::string category = path.parent_path().filename().string();

            char prefix = std::toupper(static_cast<unsigned char>(category.front()));

            int number = ++category_counters()[category];

            test_registry().push_back({std::string(1, prefix) + std::to_string(number), category,
                                       prettify(path.stem().string()), run});
        } catch (const std::exception& e) {
            std::fprintf(stderr, "Fatal: failed to register test suite from %s: %s\n", file,
                         e.what());
            std::terminate();
        } catch (...) {
            std::fprintf(stderr, "Fatal: failed to register test suite from %s: unknown error\n",
                         file);
            std::terminate();
        }
    }
};