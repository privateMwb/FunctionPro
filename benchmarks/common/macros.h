#include "helper.h"

// Executes and times a benchmark expression.
#define BENCH(name, iteration, expr) do {                      \
auto start = steady_clock::now();                              \
for (std::size_t i = 0; i < iteration; ++i) {                  \
        doNotOptimize();                                       \
        expr();                                                \
        doNotOptimize();                                       \
}                                                              \
auto end  = steady_clock::now();                               \
auto ns   = duration_cast<nanoseconds>(end - start);           \
auto raw  = formatDuration(ns);                                \
std::cout << std::left                                         \
          << std::setw(40) << prettify(name)                   \
          << std::setw(25) << timeColor(ns, raw)               \
          << std::setw(20) << iterColor(iteration)             \
          << "\n";                                             \
} while (0)


// Registers this file's run_benchmarks with the global registry so it
// runs automatically at startup. The anonymous namespace gives
// `registrar` internal linkage, preventing duplicate-symbol errors when
// this macro is expanded in multiple translation units.
#define REGISTER_BENCH_SUITE()        \
    namespace {                      \
        static BenchRegistrar registrar( \
            __FILE__,                \
            run_benchmarks                \
        );                           \
    }