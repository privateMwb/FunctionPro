// MoveOnlyFunction Examples
// Demonstrates construction, invocation, ownership,
// reassignment, and utility operations of MoveOnlyFunction.
//
// Covers:
// - Free function wrapping
// - Lambda wrapping
// - Capturing lambdas
// - Callable objects
// - Move-only callable wrapping
// - Callable reassignment
// - Move semantics
// - Reset operations
// - Empty state checks
// - String return types

#include "../include/function/MoveOnlyFunction.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <memory>

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
    MoveOnlyFunction<int(int, int)> f(add);
    print("free function:", f(2, 3));
}

// Lambda Example
// Demonstrates wrapping and invoking a lambda.
static void lambda() {
    MoveOnlyFunction<int(int)> f([](int x) { return x * 2; });
    print("lambda:", f(5));
}

// Capturing Lambda Example
// Demonstrates lambda capture support.
static void capturing_lambda() {
    int multiplier = 4;
    MoveOnlyFunction<int(int)> f([multiplier](int x) { return x * multiplier; });
    print("capturing lambda:", f(3));
}

// Callable Object Example
// Demonstrates wrapping a functor object.
static void callable_object() {
    struct Adder {
        int offset;
        int operator()(int x) const { return x + offset; }
    };
    MoveOnlyFunction<int(int)> f(Adder{10});
    print("callable object:", f(5));
}

// Move-Only Callable Example
// Demonstrates wrapping a non-copyable callable.
static void move_only_callable() {
    auto ptr = std::make_unique<int>(99);
    MoveOnlyFunction<int()> f([p = std::move(ptr)]() { return *p; });
    print("move-only callable:", f());
}

// Callable Reassignment Example
// Demonstrates replacing one callable with another.
static void reassign() {
    MoveOnlyFunction<int(int, int)> f(add);
    f = [](int a, int b) { return a * b; };
    print("reassigned:", f(3, 4));
}

// Move Construction Example
// Demonstrates ownership transfer through move construction.
static void move() {
    MoveOnlyFunction<int(int)> a([](int x) { return x * 2; });
    MoveOnlyFunction<int(int)> b(std::move(a));
    print("move:", b(7));
    print("moved-from empty:", (a == nullptr ? "true" : "false"));
}

// Move Assignment Example
// Demonstrates ownership transfer through move assignment.
static void move_assign() {
    MoveOnlyFunction<int(int)> a([](int x) { return x * 3; });
    MoveOnlyFunction<int(int)> b;
    b = std::move(a);
    print("move assign:", b(4));
    print("moved-from empty:", (a == nullptr ? "true" : "false"));
}

// Reset Example
// Demonstrates clearing the stored callable.
static void reset() {
    MoveOnlyFunction<int(int, int)> f(add);
    f.reset();
    print("after reset:", (f == nullptr ? "true" : "false"));
}

// Empty State Example
// Demonstrates checking whether a MoveOnlyFunction is empty.
static void empty_check() {
    MoveOnlyFunction<int(int)> f;
    print("empty bool:", (f ? "true" : "false"));
}

// String Return Example
// Demonstrates non-primitive return types.
static void string_return() {
    MoveOnlyFunction<std::string(std::string)> f(
        [](std::string name) {
            return "hello, " + name;
        }
    );
    print("string return:", f("world"));
}

void run_move_only_function_examples() {
    std::cout << "MoveOnlyFunction Examples\n\n";

    free_function();
    lambda();
    capturing_lambda();
    callable_object();
    move_only_callable();
    reassign();
    move();
    move_assign();
    reset();
    empty_check();
    string_return();

    std::cout << "\n";
}