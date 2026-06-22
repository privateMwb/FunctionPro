#include "test_helper.h"
#include "../include/function/FunctionRef.h"

#include <iostream>
#include <memory>
#include <stdexcept>

using namespace FunctionPro;

static void default_empty() {
    FunctionRef<int(int)> ref;
    CHK(!ref);
    CHK(ref == nullptr);
}

static void free_function() {
    FunctionRef<int(int, int)> ref(free_add);
    CHK(ref);
    CHK(ref(3, 3) == 6);
}

static void lambda() {
    int captured = 7;
    auto lam = [captured](int x) { return x + captured; };
    FunctionRef<int(int)> ref(lam);
    CHK(ref(3) == 10);
}

static void side_effect() {
    int x = 0;
    FunctionRef<void(int&)> ref(free_side);
    ref(x);
    CHK(x == 1);
}

static void throw_on_empty_call() {
    FunctionRef<int(int)> ref;
    bool threw = false;
    try { ref(1); } 
    catch (const std::bad_function_call&) { threw = true; }
    CHK(threw);
}

static void copy() {
    auto lam = [](int x) { return x * 4; };
    FunctionRef<int(int)> a(lam);
    FunctionRef<int(int)> b(a);
    CHK(b(2) == 8);
}

static void copy_assign() {
    auto lam = [](int x) { return x * 5; };
    FunctionRef<int(int)> a(lam);
    FunctionRef<int(int)> b;
    b = a;
    CHK(b(2) == 10);
}

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
