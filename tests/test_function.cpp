// Function Test Suite
// Validates construction, invocation, ownership, assignment,
// exception safety, and state management behavior of Function.
//
// Covers:
// - Empty state handling
// - Null construction
// - Free function wrapping
// - SBO callable storage
// - Heap callable storage
// - Copy semantics
// - Move semantics
// - Reset operations
// - Exception handling
// - Self-assignment
// - Callable reassignment

#include "test_helper.h"
#include "../include/function/Function.h"

#include <iostream>
#include <stdexcept>
#include <memory>

using namespace FunctionPro;

// Default Construction
// Verifies that a default-constructed Function is empty.
static void default_empty() {
    Function<int(int, int)> f;
    CHK(!f);
    CHK(f == nullptr);
}

// Null Construction
// Verifies construction from nullptr creates an empty Function.
static void null_ctor() {
    Function<int(int, int)> f(nullptr);
    CHK(!f);
}

// Free Function Invocation
// Verifies wrapping and invoking a free function.
static void free_function() {
    Function<int(int, int)> f(free_add);
    CHK(f);
    CHK(f(2, 3) == 5);
}

// SBO Lambda Storage
// Verifies small lambda storage and invocation.
static void lambda_sbo() {
    int captured = 10;
    Function<int(int)> f([captured](int x) { return x + captured; });
    CHK(f);
    CHK(f(5) == 15);
}

// Heap Lambda Storage
// Verifies large callable allocation and invocation.
static void lambda_heap() {
    struct BigCapture {
        std::byte pad[64] = {};
        int value = 99;
    } big;
    Function<int()> f([big]() { return big.value; });
    CHK(f);
    CHK(f() == 99);
}

// Copy Construction
// Verifies copied Function instances remain independent.
static void copy() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b(a);
    CHK(b(4, 6) == 10);
    CHK(a(1, 1) == 2);
}

// Copy Assignment
// Verifies assignment correctly duplicates callable state.
static void copy_assign() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b;
    b = a;
    CHK(b(3, 7) == 10);
}

// Move Construction
// Verifies ownership transfer through move construction.
static void move() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b(std::move(a));
    CHK(b(2, 2) == 4);
    CHK(!a);
}

// Move Assignment
// Verifies ownership transfer through move assignment.
static void move_assign() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b;
    b = std::move(a);
    CHK(b(5, 5) == 10);
    CHK(!a);
}

// Reset State
// Verifies reset clears the stored callable.
static void reset() {
    Function<int(int, int)> f(free_add);
    f.reset();
    CHK(!f);
    CHK(f == nullptr);
}

// Empty Invocation Exception
// Verifies calling an empty Function throws.
static void throw_on_empty_call() {
    Function<int(int, int)> f;
    bool threw = false;
    try { f(1, 2); }
    catch(const std::bad_function_call&) { threw = true; }
    CHK(threw);
}

// Self Copy Assignment
// Verifies self-copy assignment is safe.
static void self_assign_copy() {
    Function<int(int, int)> f(free_add);
    f = f;
    CHK(f(1, 2) == 3);
}

// Self Move Assignment
// Verifies self-move assignment is safe.
static void self_assign_move() {
    Function<int(int, int)> f(free_add);
    f = std::move(f);
    CHK(f(1, 2) == 3);
}

// Callable Reassignment
// Verifies replacing one callable with another.
static void reassign() {
    Function<int(int)> f([](int x) { return x * 2; });
    CHK(f(3) == 6);
    f = [](int x) { return x * 3; };
    CHK(f(3) == 9);
}

void run_function_tests() {
    std::cout << "Function Tests\n";

    RUN(default_empty);
    RUN(null_ctor);
    RUN(free_function);
    RUN(lambda_sbo);
    RUN(lambda_heap);
    RUN(copy);
    RUN(copy_assign);
    RUN(move);
    RUN(move_assign);
    RUN(reset);
    RUN(throw_on_empty_call);
    RUN(self_assign_copy);
    RUN(self_assign_move);
    RUN(reassign);

    std::cout << "\n";
}