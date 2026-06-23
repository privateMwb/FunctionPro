// FunctionRef Examples
// Demonstrates construction, invocation, rebinding,
// and utility operations of FunctionRef.
//
// Covers:
// - Free function wrapping
// - Lambda wrapping
// - Capturing lambdas
// - Callable objects
// - Void return type
// - Reference parameters
// - Copy semantics
// - Rebinding to a new callable
// - Empty state checks

#include "../include/function/FunctionRef.h"

#include <iostream>
#include <string>
#include <iomanip>

using namespace FunctionPro;

int  add(int a, int b)  { return a + b; }
void increment(int& x)  { x += 1; }

template<typename T>
void print(const std::string& title, const T& value) {
    std::cout << std::left
              << std::setw(40) << title
              << std::setw(20) << value
              << "\n";
}

// Free Function Example
// Demonstrates wrapping and invoking a free function.
static void free_function() {
    FunctionRef<int(int, int)> ref(add);
    print("free function:", ref(2, 3));
}

// Lambda Example
// Demonstrates wrapping and invoking a lambda.
static void lambda() {
    auto lam = [](int x) { return x * 2; };
    FunctionRef<int(int)> ref(lam);
    print("lambda:", ref(5));
}

// Capturing Lambda Example
// Demonstrates lambda capture support.
static void capturing_lambda() {
    int multiplier = 4;
    auto lam = [multiplier](int x) { return x * multiplier; };
    FunctionRef<int(int)> ref(lam);
    print("capturing lambda:", ref(3));
}

// Callable Object Example
// Demonstrates wrapping a functor object.
static void callable_object() {
    struct Adder {
        int offset;
        int operator()(int x) const { return x + offset; }
    };
    Adder adder{10};
    FunctionRef<int(int)> ref(adder);
    print("callable object:", ref(5));
}

// Void Return Example
// Demonstrates void return type support.
static void void_return() {
    FunctionRef<void(int&)> ref(increment);
    int x = 0;
    ref(x);
    print("void return (x after increment):", x);
}

// Reference Parameter Example
// Demonstrates passing and mutating a reference parameter.
static void reference_parameter() {
    int value = 10;
    auto lam = [](int& x) { x *= 2; };
    FunctionRef<void(int&)> ref(lam);
    ref(value);
    print("reference parameter:", value);
}

// Copy Example
// Demonstrates copying a FunctionRef — both refer to the same callable.
static void copy() {
    auto lam = [](int x) { return x * 4; };
    FunctionRef<int(int)> a(lam);
    FunctionRef<int(int)> b(a);
    print("copy:", b(2));
}

// Rebind Example
// Demonstrates reassigning a FunctionRef to a different callable.
static void rebind() {
    auto lam1 = [](int x) { return x + 1; };
    auto lam2 = [](int x) { return x + 2; };
    FunctionRef<int(int)> ref(lam1);
    print("before rebind:", ref(0));
    ref = FunctionRef<int(int)>(lam2);
    print("after rebind:", ref(0));
}

// Empty State Example
// Demonstrates checking whether a FunctionRef is empty.
static void empty_check() {
    FunctionRef<int(int)> ref;
    print("empty bool:", (ref ? "true" : "false"));
}

void run_function_ref_examples() {
    std::cout << "FunctionRef Examples\n\n";

    free_function();
    lambda();
    capturing_lambda();
    callable_object();
    void_return();
    reference_parameter();
    copy();
    rebind();
    empty_check();

    std::cout << "\n";
}