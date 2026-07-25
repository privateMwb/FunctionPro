// clang-format off
#include "helpers.h"         // doNotOptimize()
                             // getSuite()
                             // printComparisonRow()
                             // printSoloRow() SMALL/MEDIUM/LARGE
// clang-format on

// Executes and times a benchmark expression over the given number of
// iterations. Records the result into benchmark_results() when custom
// is true; does not print directly — printing is handled once per
// comparison pair by printComparisonRow(), so both sides of a
// comparison land on the same output line.
// outNs receives the elapsed duration so the caller can consume it.
#define BENCHMARK(name, iteration, expr, outNs, custom)                                            \
    do {                                                                                           \
        auto start = steady_clock::now();                                                          \
        for (std::size_t i = 0; i < iteration; ++i) {                                              \
            doNotOptimize();                                                                       \
            expr();                                                                                \
            doNotOptimize();                                                                       \
        }                                                                                          \
        auto end = steady_clock::now();                                                            \
                                                                                                   \
        (outNs) = duration_cast<nanoseconds>(end - start);                                         \
                                                                                                   \
        if (custom)                                                                                \
            benchmark_results().push_back(                                                         \
                {getSuite(), name, static_cast<std::uint64_t>((outNs).count()), iteration,         \
                 static_cast<double>((outNs).count()) / static_cast<double>(iteration)});          \
    } while (0)

// Runs a paired comparison benchmark (custom implementation vs.
// reference implementation) across the standard SMALL/MEDIUM/LARGE
// tiers (10K/100K/1M iterations), printing one comparison row per tier.
#define BENCH(name, c_expr, s_expr)                                                                \
    do {                                                                                           \
        nanoseconds cNs{}, sNs{};                                                                  \
                                                                                                   \
        BENCHMARK(std::string(name), SMALL, c_expr, cNs, true);                                    \
        BENCHMARK(std::string(name), SMALL, s_expr, sNs, false);                                   \
        printComparisonRow(name, "10K", cNs, sNs);                                                 \
                                                                                                   \
        BENCHMARK(std::string(name), MEDIUM, c_expr, cNs, true);                                   \
        BENCHMARK(std::string(name), MEDIUM, s_expr, sNs, false);                                  \
        printComparisonRow(name, "100K", cNs, sNs);                                                \
                                                                                                   \
        BENCHMARK(std::string(name), LARGE, c_expr, cNs, true);                                    \
        BENCHMARK(std::string(name), LARGE, s_expr, sNs, false);                                   \
        printComparisonRow(name, "1M", cNs, sNs);                                                  \
    } while (0)

// Runs a standalone benchmark with no reference-implementation
// equivalent (no comparison, no delta) across the standard
// SMALL/MEDIUM/LARGE tiers. Reuses BENCHMARK with custom=true so the
// result is still recorded into benchmark_results(), same as one side
// of a BENCH pair.
#define BENCH_SOLO(name, expr)                                                                     \
    do {                                                                                           \
        nanoseconds ns{};                                                                          \
                                                                                                   \
        BENCHMARK(std::string(name), SMALL, expr, ns, true);                                       \
        printSoloRow(name, "10K", ns);                                                             \
                                                                                                   \
        BENCHMARK(std::string(name), MEDIUM, expr, ns, true);                                      \
        printSoloRow(name, "100K", ns);                                                            \
                                                                                                   \
        BENCHMARK(std::string(name), LARGE, expr, ns, true);                                       \
        printSoloRow(name, "1M", ns);                                                              \
    } while (0)

// Like BENCH, but for suites whose payload is too heavy for BENCH's
// default 10K/100K/1M tiers (e.g. large structures, deep recursion) —
// at those tiers a single expensive call multiplied out would never
// finish in reasonable time. Uses 1/10/100 instead.
#define BENCH_CUSTOM(name, c_expr, s_expr)                                                         \
    do {                                                                                           \
        nanoseconds cNs{}, sNs{};                                                                  \
                                                                                                   \
        BENCHMARK(std::string(name), 1, c_expr, cNs, true);                                        \
        BENCHMARK(std::string(name), 1, s_expr, sNs, false);                                       \
        printComparisonRow(name, "1", cNs, sNs);                                                   \
                                                                                                   \
        BENCHMARK(std::string(name), 10, c_expr, cNs, true);                                       \
        BENCHMARK(std::string(name), 10, s_expr, sNs, false);                                      \
        printComparisonRow(name, "10", cNs, sNs);                                                  \
                                                                                                   \
        BENCHMARK(std::string(name), 100, c_expr, cNs, true);                                      \
        BENCHMARK(std::string(name), 100, s_expr, sNs, false);                                     \
        printComparisonRow(name, "100", cNs, sNs);                                                 \
    } while (0)

// Registers this file's run_benchmarks with the global registry so it
// runs automatically at startup. The anonymous namespace gives
// `registrar` internal linkage, preventing duplicate-symbol errors when
// this macro is expanded in multiple translation units.
#define REGISTER_BENCH_SUITE()                                                                     \
    namespace {                                                                                    \
    static BenchRegistrar registrar(__FILE__, run_benchmarks);                                     \
    }