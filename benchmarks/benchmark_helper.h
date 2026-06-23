#pragma once

#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

static const int ITERATIONS = 10'000'000;

inline void bench_header(std::string f) {
    std::string title = f + " Benchmark";
    std::cout << std::string(70, '-') << "\n";
    std::cout << std::left
        << std::setw(40) << title
        << std::setw(15) << "Time"
        << std::setw(15) << "Iterations"
        << "\n";
    std::cout << std::string(70, '-') << "\n";
}

#define BENCH(name, iterations, expr)                               \
do {                                                                \
    std::ostringstream stream;                                      \
    auto start = std::chrono::high_resolution_clock::now();         \
    for (int i = 0; i < iterations; ++i) { (void)(expr); }          \
    auto end = std::chrono::high_resolution_clock::now();           \
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>( \
                end - start).count() / (double)iterations;          \
    stream << std::fixed << std::setprecision(2) << ns;             \
    std::string time = stream.str() + " ns";                        \
    std::cout << std::left                                          \
              << std::setw(40) << name                              \
              << std::setw(15) << time                              \
              << std::setw(15) << iterations                        \
              << "\n";                                              \
} while (0)

// Shared Callables
inline int   free_add(int x, int y) { return x + y; }
inline void  free_side(int& x) { x++; }

struct SmallCallable {
    int offset = 1;
    int operator()(int a, int b) const { return a + b + offset; }
};

struct LargeCallable {
    std::byte pad[64] = {};
    int offset = 1;
    int operator()(int a, int b) const { return a + b + offset; }
};