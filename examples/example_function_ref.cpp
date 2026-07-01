// Basic FunctionRef example.
//
// Demonstrates:
// - Default construction
// - Free function wrapping
// - Lambda wrapping
// - Mutable lambda wrapping
// - Functor wrapping
// - Copy semantics
// - Rebinding
// - Passing FunctionRef by value

#include "example_helper.h"
#include <FunctionPro/FunctionRef.h>

using namespace FunctionPro;

static int free_add(int a, int b) { return a + b; }
static int free_mul(int a, int b) { return a * b; }

int main() {
    mainTitle("\nFunctionRef Examples");
    borderLine();

    // Default construction.
    setTitle("Construction");
    FunctionRef<int(int, int)> empty;
    std::cout << "Default constructed : "
              << (empty ? "non-empty" : "empty") << "\n\n";

    // Wrap a free function.
    setTitle("Free Function");
    FunctionRef<int(int, int)> f(free_add);
    std::cout << "Refers to free fn   : " << (f ? "yes" : "no") << "\n";
    std::cout << "f(3, 4)             : " << f(3, 4) << "\n\n";

    // Wrap a lambda without taking ownership.
    setTitle("Lambda");
    int bias = 10;
    auto lam = [bias](int x, int y) { return x + y + bias; };
    FunctionRef<int(int, int)> ref(lam);
    std::cout << "Refers to lambda    : " << (ref ? "yes" : "no") << "\n";
    std::cout << "ref(3, 4)           : " << ref(3, 4) << "\n\n";

    // Reference a mutable lambda.
    setTitle("Mutable Lambda");
    int counter = 0;
    auto mutable_lam = [counter](int x) mutable { return x + ++counter; };
    FunctionRef<int(int)> mref(mutable_lam);
    std::cout << "mref(10)            : " << mref(10) << "\n";
    std::cout << "mref(10)            : " << mref(10) << "\n\n";

    // Wrap a function object.
    setTitle("Functor");
    struct Adder {
        int base;
        int operator()(int x, int y) const { return x + y + base; }
    };

    Adder adder{5};
    FunctionRef<int(int, int)> fref(adder);
    std::cout << "Refers to functor   : " << (fref ? "yes" : "no") << "\n";
    std::cout << "fref(3, 4)          : " << fref(3, 4) << "\n\n";

    // Copying creates another reference to the same callable.
    setTitle("Copy");
    FunctionRef<int(int, int)> original(free_add);
    FunctionRef<int(int, int)> copied(original);
    std::cout << "Original            : " << (original ? "non-empty" : "empty") << "\n";
    std::cout << "Copied              : " << (copied ? "non-empty" : "empty") << "\n";
    std::cout << "original(2, 3)      : " << original(2, 3) << "\n";
    std::cout << "copied(2, 3)        : " << copied(2, 3) << "\n\n";

    // Rebind to a different callable.
    setTitle("Rebind");
    FunctionRef<int(int, int)> r(free_add);
    std::cout << "r(3, 4) before      : " << r(3, 4) << "\n";
    r = FunctionRef<int(int, int)>(free_mul);
    std::cout << "r(3, 4) after       : " << r(3, 4) << "\n\n";

    // Pass FunctionRef by value without copying the callable.
    setTitle("Pass By Value");
    auto invoke = [](FunctionRef<int(int, int)> fn, int a, int b) {
        return fn(a, b);
    };

    FunctionRef<int(int, int)> pref(free_add);
    std::cout << "Passed to lambda    : " << invoke(pref, 6, 7) << "\n";
    std::cout << "Passed free fn      : " << invoke(free_mul, 6, 7) << "\n";

    borderLine();
    std::cout << "\n";
    return 0;
}