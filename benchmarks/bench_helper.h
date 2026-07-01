#pragma once

#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>
#include <FunctionPro/FunctionRef.h>

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std::chrono;

// ANSI terminal color codes.
constexpr const char* RESET = "\033[0m";
constexpr const char* GREEN = "\033[92m";
constexpr const char* YELLOW = "\033[93m";
constexpr const char* RED = "\033[91m";
constexpr const char* CYAN = "\033[96m";
constexpr const char* GRAY = "\033[37m";

// Benchmark iteration presets.
constexpr const std::size_t SMALL = 100'000;
constexpr const std::size_t MEDIUM = 500'000;
constexpr const std::size_t LARGE = 1'000'000;

// Prints a horizontal separator line.
inline void borderLine() {
    std::cout << GRAY << std::string(70, '-') << RESET << "\n";
}

// Formats a duration using the most appropriate time unit.
inline auto formatDuration(nanoseconds ns) {
    std::ostringstream out;

    if (ns < microseconds(1))
        out << ns.count()
        << " ns";
    else if (ns < milliseconds(1))
        out << std::fixed << std::setprecision(2)
        << duration<double, std::micro>(ns).count()
        << " us";
    else if (ns < seconds(1))
        out << std::fixed << std::setprecision(2)
        << duration<double, std::milli>(ns).count()
        << " ms";
    else
        out << std::fixed << std::setprecision(2)
        << duration<double>(ns).count()
        << " s";

    return out.str();
}

// Converts a snake_case name to Title Case.
inline std::string prettify(const char* name) {
    std::string result{ name };
    bool firstLetter = true;

    for (char& c : result) {
        if (firstLetter) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            firstLetter = false;
        }
        if (c == '_' || c == ' ') {
            c = ' ';
            firstLetter = true;
        }
    }

    return result;
}

// Returns a colorized duration based on the elapsed time.
inline std::string timeColor(auto ns, auto raw) {
    std::string colored;

    if (ns < milliseconds(1))
        colored = GREEN + raw + RESET;
    else if (ns < seconds(1))
        colored = YELLOW + raw + RESET;
    else
        colored = RED + raw + RESET;

    return colored;
}

// Returns a colorized iteration count based on the benchmark size.
inline std::string iterColor(std::size_t iteration) {
    std::string colored;

    if (iteration <= 100'000)
        colored = GREEN + std::to_string(iteration) + RESET;
    else if (iteration <= 500'000)
        colored = YELLOW + std::to_string(iteration) + RESET;
    else
        colored = RED + std::to_string(iteration) + RESET;

    return colored;
}

// Prevents the compiler from optimizing away benchmarked values.
template<typename T>
inline void doNotOptimize(T& value) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "g"(value) : "memory");
#else
    volatile const T* p = &value;
    (void)p;
#endif
}

// Prevents the compiler from optimizing away benchmarked code.
inline void doNotOptimize() {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" ::: "memory");
#endif
}

// Prints the benchmark table header.
inline void setHeader(const char* header) {
    borderLine();
    std::cout << std::left << CYAN
        << std::setw(40) << prettify(header)
        << std::setw(15) << "Time"
        << std::setw(15) << "Iteration"
        << RESET << "\n";
    borderLine();
}

inline void setSubHeader(const char* header) {
    std::cout << std::left << CYAN
        << std::setw(40) << prettify(header)
        << RESET << "\n";        
}

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