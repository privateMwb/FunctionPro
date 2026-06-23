// MoveOnlyFunction Test Suite
// Validates move-only callable storage, invocation, ownership transfer,
// reassignment, reset behavior, and exception safety.
//
// Covers:
// - Empty state handling
// - Free function wrapping
// - Move-only lambda captures
// - Move construction
// - Move assignment
// - Reset operations
// - Exception handling
// - Self move-assignment
// - Callable reassignment

#include "test_helper.h"
#include "../include/function/MoveOnlyFunction.h"

#include <iostream>
#include <memory>
#include <stdexcept>

using namespace FunctionPro;

// Default Construction
// Verifies that a default-constructed wrapper is empty.
static void default_empty() {
    MoveOnlyFunction<int(int, int)> f;
    CHK(!f);
    CHK(f == nullptr);
}

// Free Function Invocation
// Verifies wrapping and invoking a free function.
static void basic() {
    MoveOnlyFunction<int(int, int)> f(free_add);
    CHK(f);
    CHK(f(3, 4) == 7);
}

// Move-Only Lambda Capture
// Verifies support for move-only resources such as unique_ptr.
static void move_only_lambda() {
    auto ptr = std::make_unique<int>(55);
    MoveOnlyFunction<int()> f(
        [p = std::move(ptr)]() {
            return *p;
        }
    );
    CHK(f() == 55);
}

// Move Construction
// Verifies ownership transfer through move construction.
static void move() {
    MoveOnlyFunction<int(int, int)> a(free_add);
    MoveOnlyFunction<int(int, int)> b(std::move(a));
    CHK(b(6, 4) == 10);
    CHK(!a);
}

// Move Assignment
// Verifies ownership transfer through move assignment.
static void move_assign() {
    MoveOnlyFunction<int(int, int)> a(free_add);
    MoveOnlyFunction<int(int, int)> b;
    b = std::move(a);
    CHK(b(2, 8) == 10);
    CHK(!a);
}

// Reset State
// Verifies reset clears the stored callable.
static void reset() {
    MoveOnlyFunction<int(int, int)> f(free_add);
    f.reset();
    CHK(!f);
    CHK(f == nullptr);
}

// Empty Invocation Exception
// Verifies calling an empty wrapper throws an exception.
static void throw_on_empty_call() {
    MoveOnlyFunction<int(int)> f;
    bool threw = false;
    try { f(1); }
    catch (const std::bad_function_call&) { threw = true; }
    CHK(threw);
}

// Self Move Assignment
// Verifies self-move assignment remains safe and valid.
static void self_assign_move() {
    MoveOnlyFunction<int(int, int)> f(free_add);
    f = std::move(f);
    CHK(f(1, 2) == 3);
}

// Callable Reassignment
// Verifies replacing one callable with another.
static void reassign() {
    MoveOnlyFunction<int(int)> f([](int x) { return x * 2; } );
    CHK(f(3) == 6);
    f = [](int x) { return x * 3; };
    CHK(f(3) == 9);
}

void run_move_only_function_tests() {
    std::cout << "\nMoveOnlyFunction Tests\n";

    RUN(default_empty);
    RUN(basic);
    RUN(move_only_lambda);
    RUN(move);
    RUN(move_assign);
    RUN(reset);
    RUN(throw_on_empty_call);
    RUN(self_assign_move);
    RUN(reassign);

    std::cout << "\n";
}