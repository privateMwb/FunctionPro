// MoveOnlyFunction Test Suite
// Validates construction, move semantics, invocation,
// state management, and swapping behavior.
//
// Covers:
// - default and nullptr construction
// - free functions
// - SBO and heap-stored lambdas
// - move-only callables
// - move semantics
// - reset behavior
// - empty invocation
// - self move-assignment
// - callable reassignment
// - member and free swap
// - non-copyable guarantees

#include "test_helper.h"

#include <iostream>
#include <memory>
#include <stdexcept>

using namespace FunctionPro;

static int free_add(int a, int b) { return a + b; }

// Verifies default construction creates an empty MoveOnlyFunction.
static void default_empty() {
    MoveOnlyFunction<int(int, int)> f;
    CHK(!f);
    CHK(f == nullptr);
    CHK(f != nullptr == false);
}

// Verifies nullptr construction creates an empty MoveOnlyFunction.
static void null_ctor() {
    MoveOnlyFunction<int(int, int)> f(nullptr);
    CHK(!f);
    CHK(f == nullptr);
}

// Verifies free functions can be stored and invoked.
static void free_function() {
    MoveOnlyFunction<int(int, int)> f(free_add);
    CHK(f);
    CHK(f != nullptr);
    CHK(f(2, 3) == 5);
}

// Verifies small lambdas use Small Buffer Optimization.
static void lambda_sbo() {
    int captured = 10;
    MoveOnlyFunction<int(int)> f([captured](int x) { return x + captured; });

    CHK(f);
    CHK(f(5) == 15);
}

// Verifies large lambdas are heap allocated.
static void lambda_heap() {
    struct BigCapture {
        std::byte pad[64] = {};
        int value = 99;
    } big;

    MoveOnlyFunction<int()> f([big]() { return big.value; });

    CHK(f);
    CHK(f() == 99);
}

// Verifies move-only callables can be stored and invoked.
static void move_only_callable() {
    auto ptr = std::make_unique<int>(42);

    MoveOnlyFunction<int()> f([p = std::move(ptr)]() { return *p; });

    CHK(f);
    CHK(f() == 42);
}

// Verifies move construction transfers SBO callables.
static void move_sbo() {
    MoveOnlyFunction<int(int, int)> a(free_add);
    MoveOnlyFunction<int(int, int)> b(std::move(a));

    CHK(b(2, 2) == 4);
    CHK(!a);
}

// Verifies move construction transfers heap-stored callables.
static void move_heap() {
    struct BigCapture {
        std::byte pad[64] = {};
        int value = 55;
    } big;

    MoveOnlyFunction<int()> a([big]() { return big.value; });
    MoveOnlyFunction<int()> b(std::move(a));

    CHK(b() == 55);
    CHK(!a);
}

// Verifies move assignment transfers SBO callables.
static void move_assign_sbo() {
    MoveOnlyFunction<int(int, int)> a(free_add);
    MoveOnlyFunction<int(int, int)> b;

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

    MoveOnlyFunction<int()> a([big]() { return big.value; });
    MoveOnlyFunction<int()> b;

    b = std::move(a);

    CHK(b() == 33);
    CHK(!a);
}

// Verifies heap-allocated move-only callables transfer correctly.
static void move_only_unique_ptr_heap() {
    auto ptr = std::make_unique<std::byte[]>(64);
    int* val = reinterpret_cast<int*>(ptr.get());
    *val = 77;

    MoveOnlyFunction<int()> a([p = std::move(ptr), val]() { return *val; });
    MoveOnlyFunction<int()> b(std::move(a));

    CHK(b() == 77);
    CHK(!a);
}

// Verifies reset clears SBO callables.
static void reset_sbo() {
    MoveOnlyFunction<int(int, int)> f(free_add);

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

    MoveOnlyFunction<int()> f([big]() { return big.value; });

    f.reset();

    CHK(!f);
    CHK(f == nullptr);
}

// Verifies reset releases move-only callables.
static void reset_move_only() {
    auto ptr = std::make_unique<int>(99);

    MoveOnlyFunction<int()> f([p = std::move(ptr)]() { return *p; });

    f.reset();

    CHK(!f);
}

// Verifies invoking an empty MoveOnlyFunction throws.
static void throw_on_empty_call() {
    MoveOnlyFunction<int(int, int)> f;

    bool threw = false;

    try {
        f(1, 2);
    } catch (const std::bad_function_call&) {
        threw = true;
    }

    CHK(threw);
}

// Verifies move self-assignment is safe.
static void self_assign_move() {
    MoveOnlyFunction<int(int, int)> f(free_add);

    f = std::move(f);

    CHK(f);
    CHK(f(1, 2) == 3);
}

// Verifies assigning a new callable replaces the previous one.
static void reassign() {
    MoveOnlyFunction<int(int)> f([](int x) { return x * 2; });

    CHK(f(3) == 6);

    f = [](int x) { return x * 3; };

    CHK(f(3) == 9);
}

// Verifies move-only callables can be reassigned.
static void reassign_move_only() {
    auto p1 = std::make_unique<int>(2);
    auto p2 = std::make_unique<int>(3);

    MoveOnlyFunction<int(int)> f([p = std::move(p1)](int x) { return x * *p; });

    CHK(f(3) == 6);

    f = [p = std::move(p2)](int x) { return x * *p; };

    CHK(f(3) == 9);
}

// Verifies the member swap exchanges stored callables.
static void swap_member() {
    MoveOnlyFunction<int(int, int)> a(free_add);
    MoveOnlyFunction<int(int, int)> b([](int x, int y) { return x * y; });

    a.swap(b);

    CHK(a(3, 4) == 12);
    CHK(b(3, 4) == 7);
}

// Verifies the free swap exchanges stored callables.
static void swap_free() {
    MoveOnlyFunction<int(int, int)> a(free_add);
    MoveOnlyFunction<int(int, int)> b([](int x, int y) { return x * y; });

    swap(a, b);

    CHK(a(3, 4) == 12);
    CHK(b(3, 4) == 7);
}

// Verifies swapping with an empty MoveOnlyFunction transfers ownership.
static void swap_with_empty() {
    MoveOnlyFunction<int(int, int)> a(free_add);
    MoveOnlyFunction<int(int, int)> b;

    a.swap(b);

    CHK(!a);
    CHK(b(1, 2) == 3);
}

// Verifies MoveOnlyFunction cannot be copied.
static void not_copyable() {
    CHK(!std::is_copy_constructible_v<MoveOnlyFunction<int()>>);
    CHK(!std::is_copy_assignable_v<MoveOnlyFunction<int()>>);
}

// Executes all MoveOnlyFunction test cases.
void run_move_only_function_tests() {
    setTitle("MoveOnlyFunction Tests");

    RUN(default_empty);
    RUN(null_ctor);
    RUN(free_function);
    RUN(lambda_sbo);
    RUN(lambda_heap);
    RUN(move_only_callable);
    RUN(move_sbo);
    RUN(move_heap);
    RUN(move_assign_sbo);
    RUN(move_assign_heap);
    RUN(move_only_unique_ptr_heap);
    RUN(reset_sbo);
    RUN(reset_heap);
    RUN(reset_move_only);
    RUN(throw_on_empty_call);
    RUN(self_assign_move);
    RUN(reassign);
    RUN(reassign_move_only);
    RUN(swap_member);
    RUN(swap_free);
    RUN(swap_with_empty);
    RUN(not_copyable);

    std::cout << "\n";
}