// Function Test Suite
// Validates construction, ownership semantics, invocation,
// state management, and swapping behavior.
//
// Covers:
// - default and nullptr construction
// - free functions
// - SBO and heap-stored lambdas
// - copy and move semantics
// - reset behavior
// - empty invocation
// - self-assignment
// - callable reassignment
// - member and free swap

#include "test_helper.h"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace FunctionPro;

static int free_add(int a, int b) { return a + b; }

// Verifies default construction creates an empty Function.
static void default_empty() {
    Function<int(int, int)> f;
    CHK(!f);
    CHK(f == nullptr);
    CHK(f != nullptr == false);
}

// Verifies nullptr construction creates an empty Function.
static void null_ctor() {
    Function<int(int, int)> f(nullptr);
    CHK(!f);
    CHK(f == nullptr);
}

// Verifies free functions can be stored and invoked.
static void free_function() {
    Function<int(int, int)> f(free_add);
    CHK(f);
    CHK(f != nullptr);
    CHK(f(2, 3) == 5);
}

// Verifies small lambdas use Small Buffer Optimization.
static void lambda_sbo() {
    int captured = 10;
    Function<int(int)> f([captured](int x) { return x + captured; });
    CHK(f);
    CHK(f(5) == 15);
}

// Verifies large lambdas are heap allocated.
static void lambda_heap() {
    struct BigCapture {
        std::byte pad[64] = {};
        int value = 99;
    } big;

    Function<int()> f([big]() { return big.value; });

    CHK(f);
    CHK(f() == 99);
}

// Verifies copy construction duplicates SBO callables.
static void copy_sbo() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b(a);

    CHK(b(4, 6) == 10);
    CHK(a(1, 1) == 2);
}

// Verifies copy construction duplicates heap-stored callables.
static void copy_heap() {
    struct BigCapture {
        std::byte pad[64] = {};
        int value = 42;
    } big;

    Function<int()> a([big]() { return big.value; });
    Function<int()> b(a);

    CHK(a() == 42);
    CHK(b() == 42);
}

// Verifies copy assignment duplicates SBO callables.
static void copy_assign_sbo() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b;

    b = a;

    CHK(b(3, 7) == 10);
    CHK(a(1, 1) == 2);
}

// Verifies copy assignment duplicates heap-stored callables.
static void copy_assign_heap() {
    struct BigCapture {
        std::byte pad[64] = {};
        int value = 7;
    } big;

    Function<int()> a([big]() { return big.value; });
    Function<int()> b;

    b = a;

    CHK(a() == 7);
    CHK(b() == 7);
}

// Verifies move construction transfers SBO callables.
static void move_sbo() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b(std::move(a));

    CHK(b(2, 2) == 4);
    CHK(!a);
}

// Verifies move construction transfers heap-stored callables.
static void move_heap() {
    struct BigCapture {
        std::byte pad[64] = {};
        int value = 55;
    } big;

    Function<int()> a([big]() { return big.value; });
    Function<int()> b(std::move(a));

    CHK(b() == 55);
    CHK(!a);
}

// Verifies move assignment transfers SBO callables.
static void move_assign_sbo() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b;

    b = std::move(a);

    CHK(b(5, 5) == 10);
    CHK(!a);
}

// Verifies move assignment transfers heap-stored callables.
static void move_assign_heap() {
    struct BigCapture {
        std::byte pad[64] = {};
        int value = 33;
    } big;

    Function<int()> a([big]() { return big.value; });
    Function<int()> b;

    b = std::move(a);

    CHK(b() == 33);
    CHK(!a);
}

// Verifies reset clears SBO callables.
static void reset_sbo() {
    Function<int(int, int)> f(free_add);

    f.reset();

    CHK(!f);
    CHK(f == nullptr);
}

// Verifies reset clears heap-stored callables.
static void reset_heap() {
    struct BigCapture {
        std::byte pad[64] = {};
        int value = 1;
    } big;

    Function<int()> f([big]() { return big.value; });

    f.reset();

    CHK(!f);
    CHK(f == nullptr);
}

// Verifies invoking an empty Function throws.
static void throw_on_empty_call() {
    Function<int(int, int)> f;

    bool threw = false;

    try {
        f(1, 2);
    } catch (const std::bad_function_call&) {
        threw = true;
    }

    CHK(threw);
}

// Verifies copy self-assignment is safe.
static void self_assign_copy() {
    Function<int(int, int)> f(free_add);

    f = f;

    CHK(f);
    CHK(f(1, 2) == 3);
}

// Verifies move self-assignment is safe.
static void self_assign_move() {
    Function<int(int, int)> f(free_add);

    f = std::move(f);

    CHK(f);
    CHK(f(1, 2) == 3);
}

// Verifies assigning a new callable replaces the previous one.
static void reassign() {
    Function<int(int)> f([](int x) { return x * 2; });

    CHK(f(3) == 6);

    f = [](int x) { return x * 3; };

    CHK(f(3) == 9);
}

// Verifies the member swap exchanges stored callables.
static void swap_member() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b([](int x, int y) { return x * y; });

    a.swap(b);

    CHK(a(3, 4) == 12);
    CHK(b(3, 4) == 7);
}

// Verifies the free swap exchanges stored callables.
static void swap_free() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b([](int x, int y) { return x * y; });

    swap(a, b);

    CHK(a(3, 4) == 12);
    CHK(b(3, 4) == 7);
}

// Verifies swapping with an empty Function transfers ownership.
static void swap_with_empty() {
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b;

    a.swap(b);

    CHK(!a);
    CHK(b(1, 2) == 3);
}

// Executes all Function test cases.
void run_function_tests() {
    setTitle("Function Tests");

    RUN(default_empty);
    RUN(null_ctor);
    RUN(free_function);
    RUN(lambda_sbo);
    RUN(lambda_heap);
    RUN(copy_sbo);
    RUN(copy_heap);
    RUN(copy_assign_sbo);
    RUN(copy_assign_heap);
    RUN(move_sbo);
    RUN(move_heap);
    RUN(move_assign_sbo);
    RUN(move_assign_heap);
    RUN(reset_sbo);
    RUN(reset_heap);
    RUN(throw_on_empty_call);
    RUN(self_assign_copy);
    RUN(self_assign_move);
    RUN(reassign);
    RUN(swap_member);
    RUN(swap_free);
    RUN(swap_with_empty);

    std::cout << "\n";
}