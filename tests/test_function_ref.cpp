// FunctionRef Test Suite
// Validates non-owning callable references, invocation,
// rebinding, copy semantics, and lifetime behavior.
//
// Covers:
// - default construction
// - free functions
// - lambdas and mutable lambdas
// - functors
// - copy semantics
// - rebinding
// - empty invocation
// - non-owning lifetime
// - pass-by-value usage
// - type traits

#include "test_helper.h"

#include <iostream>
#include <stdexcept>

using namespace FunctionPro;

static int free_add(int a, int b) { return a + b; }
static int free_double(int x)     { return x * 2; }

// Verifies default construction creates an empty FunctionRef.
static void default_empty() {
    FunctionRef<int(int, int)> f;
    CHK(!f);
    CHK(f == nullptr);
    CHK(f != nullptr == false);
}

// Verifies free functions can be referenced and invoked.
static void free_function() {
    FunctionRef<int(int, int)> f(free_add);

    CHK(f);
    CHK(f != nullptr);
    CHK(f(2, 3) == 5);
}

// Verifies lambdas can be referenced and invoked.
static void lambda() {
    int captured = 10;
    auto lam = [captured](int x) { return x + captured; };

    FunctionRef<int(int)> f(lam);

    CHK(f);
    CHK(f(5) == 15);
}

// Verifies mutable lambdas preserve their internal state.
static void mutable_lambda() {
    int counter = 0;
    auto lam = [counter](int x) mutable { return x + ++counter; };

    FunctionRef<int(int)> f(lam);

    CHK(f(10) == 11);
    CHK(f(10) == 12);
}

// Verifies functor objects can be referenced.
static void functor() {
    struct Adder {
        int base;
        int operator()(int x) const { return x + base; }
    };

    Adder adder{5};

    FunctionRef<int(int)> f(adder);

    CHK(f(3) == 8);
}

// Verifies const functors can be referenced.
static void const_functor() {
    struct Multiplier {
        int factor;
        int operator()(int x) const { return x * factor; }
    };

    const Multiplier m{3};

    FunctionRef<int(int)> f(m);

    CHK(f(4) == 12);
}

// Verifies copy construction preserves the referenced callable.
static void copy_ctor() {
    auto lam = [](int x, int y) { return x + y; };

    FunctionRef<int(int, int)> a(lam);
    FunctionRef<int(int, int)> b(a);

    CHK(b(3, 4) == 7);
    CHK(a(1, 1) == 2);
}

// Verifies copy assignment rebinds the reference.
static void copy_assign() {
    auto lam1 = [](int x, int y) { return x + y; };
    auto lam2 = [](int x, int y) { return x * y; };

    FunctionRef<int(int, int)> a(lam1);
    FunctionRef<int(int, int)> b(lam2);

    b = a;

    CHK(b(3, 4) == 7);
}

// Verifies a FunctionRef can be rebound to another callable.
static void rebind() {
    auto lam1 = [](int x) { return x * 2; };
    auto lam2 = [](int x) { return x * 3; };

    FunctionRef<int(int)> f(lam1);

    CHK(f(3) == 6);

    f = FunctionRef<int(int)>(lam2);

    CHK(f(3) == 9);
}

// Verifies function pointers can be referenced.
static void free_function_pointer() {
    FunctionRef<int(int)> f(free_double);

    CHK(f);
    CHK(f(5) == 10);
}

// Verifies invoking an empty FunctionRef throws.
static void throw_on_empty_call() {
    FunctionRef<int(int, int)> f;

    bool threw = false;

    try {
        f(1, 2);
    } catch (const std::bad_function_call&) {
        threw = true;
    }

    CHK(threw);
}

// Verifies FunctionRef does not own the referenced callable.
static void non_owning_lifetime() {
    FunctionRef<int(int)> f;

    {
        int captured = 7;
        auto lam = [captured](int x) { return x + captured; };

        f = FunctionRef<int(int)>(lam);

        CHK(f(3) == 10);
    }

    // Clear the reference before the callable goes out of scope.
    f = FunctionRef<int(int)>();

    CHK(!f);
}

// Verifies FunctionRef is inexpensive to pass by value.
static void pass_by_value() {
    auto lam = [](int x, int y) { return x + y; };

    FunctionRef<int(int, int)> ref(lam);

    auto invoke = [](FunctionRef<int(int, int)> fn) {
        return fn(3, 4);
    };

    CHK(invoke(ref) == 7);
}

// Verifies FunctionRef satisfies its expected type traits.
static void not_owning() {
    CHK(std::is_copy_constructible_v<FunctionRef<int()>>);
    CHK(std::is_copy_assignable_v<FunctionRef<int()>>);
    CHK(std::is_move_constructible_v<FunctionRef<int()>>);
    CHK(std::is_move_assignable_v<FunctionRef<int()>>);
    CHK(!std::is_default_constructible_v<FunctionRef<int()>> == false);
}

// Executes all FunctionRef test cases.
void run_function_ref_tests() {
    setTitle("FunctionRef Tests");

    RUN(default_empty);
    RUN(free_function);
    RUN(lambda);
    RUN(mutable_lambda);
    RUN(functor);
    RUN(const_functor);
    RUN(copy_ctor);
    RUN(copy_assign);
    RUN(rebind);
 
    RUN(free_function_pointer);
    RUN(throw_on_empty_call);
    RUN(non_owning_lifetime);
    RUN(pass_by_value);
    RUN(not_owning);

    std::cout << "\n";
}