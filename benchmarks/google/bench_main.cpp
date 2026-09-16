#include <benchmark/benchmark.h>

// Google Benchmark's own CLI flags (--benchmark_filter=<regex>,
// --benchmark_format=json, --benchmark_out=<path>, etc.) are wired up by
// Initialize()/RunSpecifiedBenchmarks() below. Unlike custom/bench_main.cpp,
// there's no list/category/id dispatch to write here — Google Benchmark
// already provides its own filtering via --benchmark_filter, so this file
// stays as the framework's standard boilerplate entry point.
int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);

    if (benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}