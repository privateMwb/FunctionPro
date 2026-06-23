// Function Examples
// Demonstrates construction, invocation, ownership,
// reassignment, and utility operations of Function.
//
// Covers:
// - Free function wrapping
// - Lambda wrapping
// - Capturing lambdas
// - Callable objects
// - Callable reassignment
// - Copy semantics
// - Move semantics
// - Reset operations
// - Empty state checks
// - String return types

#include "../include/function/Function.h"

#include <iostream>
#include <string>
#include <iomanip>

using namespace FunctionPro;

static int add(int a, int b) { return a + b; }

template<typename T>
static void print(const std::string& title, const T& value) {
    std::cout << std::left
              << std::setw(40) << title
              << std::setw(20) << value
              << "\n";
}

// Free Function Example
// Demonstrates wrapping and invoking a free function.
static void free_function() {
    Function<int(int, int)> f(add);
    print("free function:", f(2, 3));
}

// Lambda Example
// Demonstrates wrapping and invoking a lambda.
static void lambda() {
    Function<int(int)> f([](int x) { return x * 2; });
    print("lambda:", f(5));
}

// Capturing Lambda Example
// Demonstrates lambda capture support.
static void capturing_lambda() {
    int multiplier = 4;
    Function<int(int)> f([multiplier](int x) { return x * multiplier; });
    print("capturing lambda:", f(3));
}

// Callable Object Example
// Demonstrates wrapping a functor object.
static void callable_object() {
    struct Adder {
        int offset;
        int operator()(int x) const { return x + offset; }
    };
    Function<int(int)> f(Adder{10});
    print("callable object:", f(5));
}

// Callable Reassignment Example
// Demonstrates replacing one callable with another.
static void reassign() {
    Function<int(int, int)> f(add);
    f = [](int a, int b) {return a * b; };
    print("reassigned:", f(3, 4));
}

// Copy Construction Example
// Demonstrates copying a Function instance.
static void copy() {
    Function<int(int)> a([](int x) { return x * 2; });
    Function<int(int)> b(a);
    print("copy:", b(6));
}

// Move Construction Example
// Demonstrates ownership transfer through move construction.
static void move() {
    Function<int(int)> a([](int x) { return x * 2; });
    Function<int(int)> b(std::move(a));
    print("move:", b(7));
    print("moved-from empty:", (a == nullptr ? "true" : "false"));
}

// Reset Example
// Demonstrates clearing the stored callable.
static void reset() {
    Function<int(int, int)> f(add);
    f.reset();
    print("after reset:", (f == nullptr ? "true" : "false"));
}

// Empty State Example
// Demonstrates checking whether a Function is empty.
static void empty_check() {
    Function<int(int)> f;
    print("empty bool:", (f ? "true" : "false"));
}

// String Return Example
// Demonstrates non-primitive return types.
static void string_return() {
    Function<std::string(std::string)> f(
        [](std::string name) {
            return "hello, " + name;
        }
    );

    print("string return:", f("world"));
}

void run_function_examples() {
    std::cout << "Function Examples\n\n";

    free_function();
    lambda();
    capturing_lambda();
    callable_object();
    reassign();
    copy();
    move();
    reset();
    empty_check();
    string_return();

    std::cout << "\n";
}