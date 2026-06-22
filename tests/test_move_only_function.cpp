#include "test_helper.h"
#include "../include/function/MoveOnlyFunction.h"

#include <iostream>
#include <memory>
#include <stdexcept>

using namespace FunctionPro;

static void default_empty() {
    MoveOnlyFunction<int(int, int)> f;
    CHK(!f);
    CHK(f == nullptr);
}

static void basic() {
    MoveOnlyFunction<int(int, int)> f(free_add);
    CHK(f);
    CHK(f(3, 4) == 7);
}

static void move_only_lambda() {
    auto ptr = std::make_unique<int>(55);
    MoveOnlyFunction<int()> f([p = std::move(ptr)]() { return *p; });
    CHK(f() == 55);
}

static void move() {
    MoveOnlyFunction<int(int, int)> a(free_add);
    MoveOnlyFunction<int(int, int)> b(std::move(a));
    CHK(b(6, 4) == 10);
    CHK(!a);
}

static void move_assign() {
    MoveOnlyFunction<int(int, int)> a(free_add);
    MoveOnlyFunction<int(int, int)> b;
    b = std::move(a);
    CHK(b(2, 8) == 10);
    CHK(!a);
}

static void reset() {
    MoveOnlyFunction<int(int, int)> f(free_add);
    f.reset();
    CHK(!f);
    CHK(f == nullptr);
}

static void throw_on_empty_call() {
    MoveOnlyFunction<int(int)> f;
    bool threw = false;
    try { f(1); } 
    catch (const std::bad_function_call&) { threw = true; }
    CHK(threw);
}

static void self_assign_move() {
    MoveOnlyFunction<int(int, int)> f(free_add);
    f = std::move(f);
    CHK(f(1, 2) == 3);
}

static void reassign() {
    MoveOnlyFunction<int(int)> f([](int x) { return x * 2; });
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
