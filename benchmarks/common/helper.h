#pragma once

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

// Converts a snake_case function name to Title Case.
inline std::string prettify(std::string_view text)
{
    std::string result{text};
    bool firstLetter = true;

    for (char& c : result) {
        if (firstLetter) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            firstLetter = false;
        } else if (c == '_') {
            c = ' ';
            firstLetter = true;
        }
    }

    return result;
}

// Returns a colorized duration based on the elapsed time.
// Only distinguishes ms/s granularity (unlike formatDuration's ns/us split),
// since sub-millisecond timings are all "fast" for coloring purposes.
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
inline void doNotOptimize(const T& value) {
#if defined(__GNUC__) || defined(__clang__)
    // "g" constraint forces the value into a register/memory and the
    // "memory" clobber stops the compiler from reordering or eliding
    // the computation that produced it.
    asm volatile("" : : "g"(value) : "memory");
#else
    // Weaker fallback for compilers without inline asm support: taking
    // a volatile pointer discourages (but doesn't guarantee against)
    // the optimizer dropping the computation.
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

// Transform string case to lower case.
inline std::string toLower(std::string_view str) {
    std::string result(str);

    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return result;
}

// Prints the benchmark table header.
inline void setHeader(std::string_view header) {
    borderLine();
    std::cout << std::left << CYAN
        << std::setw(40) << prettify(header)
        << std::setw(15) << "Time"
        << std::setw(15) << "Iteration"
        << RESET << "\n";
    borderLine();
}

inline void setSubHeader(std::string_view header) {
    std::cout << std::left << CYAN
        << std::setw(40) << prettify(header)
        << RESET << "\n";        
}