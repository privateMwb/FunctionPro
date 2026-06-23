// FunctionRef Test Suite
// Validates non-owning callable references, invocation behavior,
// rebinding, copying, and exception safety.
//
// Covers:
// - Empty state handling
// - Free function binding
// - Lambda binding
// - Side-effect invocation
// - Exception handling
// - Copy construction
// - Copy assignment
// - Reference rebinding

#include "test_helper.h"
#include "../include/function/FunctionRef.h"

#include <iostream>
#include <memory>
#include <stdexcept>

using namespace FunctionPro;

// Default Construction
// Verifies that a default-constructed FunctionRef is empty.
static void default_empty() {
    FunctionRef<int(int)> ref;
    CHK(!ref);
    CHK(ref == nullptr);
}

// Free Function Binding
// Verifies binding and invoking a free function.
static void free_function() {
    FunctionRef<int(int, int)> ref(free_add);
    CHK(ref);
    CHK(ref(3, 3) == 6);
}

// Lambda Binding
// Verifies binding and invoking a lambda.
static void lambda() {
    int captured = 7;
    auto lam = [captured](int x) { return x + captured; };
    FunctionRef<int(int)> ref(lam);
    CHK(ref(3) == 10);
}

// Side Effect Invocation
// Verifies callables can modify referenced arguments.
static void side_effect() {
    int x = 0;
    FunctionRef<void(int&)> ref(free_side);
    ref(x);
    CHK(x == 1);
}

// Empty Invocation Exception
// Verifies calling an empty FunctionRef throws.
static void throw_on_empty_call() {
    FunctionRef<int(int)> ref;
    bool threw = false;
    try { ref(1); }
    catch (const std::bad_function_call&) { threw = true; }
    CHK(threw);
}

// Copy Construction
// Verifies copied references point to the same callable target.
static void copy() {
    auto lam = [](int x) { return x * 4; };
    FunctionRef<int(int)> a(lam);
    FunctionRef<int(int)> b(a);
    CHK(b(2) == 8);
}

// Copy Assignment
// Verifies copy assignment preserves callable binding.
static void copy_assign() {
    auto lam = [](int x) { return x * 5; };
    FunctionRef<int(int)> a(lam);
    FunctionRef<int(int)> b;
    b = a;
    CHK(b(2) == 10);
}

// Reference Rebinding
// Verifies a FunctionRef can be rebound to another callable.
static void rebind() {
    auto lam1 = [](int x) { return x + 1; };
    auto lam2 = [](int x) { return x + 2; };
    FunctionRef<int(int)> ref(lam1);
    CHK(ref(0) == 1);
    ref = FunctionRef<int(int)>(lam2);
    CHK(ref(0) == 2);
}

void run_function_ref_tests() {
    std::cout << "\nFunctionRef Tests\n";

    RUN(default_empty);
    RUN(free_function);
    RUN(lambda);
    RUN(side_effect);
    RUN(throw_on_empty_call);
    RUN(copy);
    RUN(copy_assign);
    RUN(rebind);

    std::cout << "\n";
}