#pragma once

// clang-format off
#include "helpers.h"         // prettify()

#include <cctype>            // std::toupper
#include <filesystem>        // std::filesystem::path
#include <string>            // std::string
#include <unordered_map>     // std::unordered_map
#include <vector>            // std::vector
// clang-format on

// One registered benchmark suite: an id ("B1"), the category it belongs to
// (derived from its containing directory), a display name, and the
// function that runs it. Populated by BenchRegistrar's constructor via
// REGISTER_BENCH_SUITE(), not constructed directly.
struct BenchSuite {
    std::string id;
    std::string category;
    std::string name;
    void (*run)();
};

// Global list of every registered benchmark suite, in registration order.
inline std::vector<BenchSuite>& bench_registry() {
    static std::vector<BenchSuite> registry;
    return registry;
}

// Per-category counter used to number suites within their category
// (e.g. the 2nd suite under "search/" becomes "S2").
inline std::unordered_map<std::string, int>& category_counters() {
    static std::unordered_map<std::string, int> counters;
    return counters;
}

// Registers one benchmark suite at static-init time. Construct via
// REGISTER_BENCH_SUITE() in a .cpp file, not directly — the constructor
// derives the suite's category from `file`'s containing directory name
// and assigns it the next number within that category.
struct BenchRegistrar {
    BenchRegistrar(const char* file, void (*run)()) {
        auto path = std::filesystem::path(file);

        std::string category = path.parent_path().filename().string();

        char prefix = std::toupper(static_cast<unsigned char>(category.front()));

        int number = ++category_counters()[category];

        bench_registry().push_back({std::string(1, prefix) + std::to_string(number), category,
                                    prettify(path.stem().string()), run});
    }
};

// One recorded benchmark measurement: which suite/operation it's from,
// total elapsed time, iteration count, and the derived per-iteration cost.
// Shape must match what exportJson()/loadResults() (regression tool) read
// and write — see export.h.
struct BenchmarkResult {
    std::string suite;
    std::string operation;
    std::uint64_t total_ns;
    std::size_t iterations;
    double ns_per_op;
};

// Global list of every benchmark result recorded this run, in the order
// BENCHMARK()/BENCH()/BENCH_SOLO() produced them. Read by exportJson() and
// grouped-by-suite by printComparisonRow()/printSoloRow() as they print.
inline std::vector<BenchmarkResult>& benchmark_results() {
    static std::vector<BenchmarkResult> results;
    return results;
}