// Function Example
// Demonstrates common Function operations, including construction,
// invocation, ownership transfer, state management, and swapping.
//
// Covers:
// - default and nullptr construction
// - free function wrapping
// - SBO lambda wrapping
// - heap lambda wrapping
// - copy semantics
// - move semantics
// - reset and reassignment
// - swap

#include "example_helper.h"
#include <FunctionPro/Function.h>

using namespace FunctionPro;

// Free function used throughout the examples.
static int free_add(int a, int b) { return a + b; }

int main() {
    mainTitle("\nFunction Examples");
    borderLine();

    // Demonstrates default and nullptr construction.
    setTitle("Construction");
    Function<int(int, int)> empty;
    std::cout << "Default constructed : " << (empty ? "non-empty" : "empty") << "\n";
    Function<int(int, int)> from_null(nullptr);
    std::cout << "From nullptr        : " << (from_null ? "non-empty" : "empty") << "\n\n";

    // Demonstrates wrapping a free function.
    setTitle("Free Function");
    Function<int(int, int)> f(free_add);
    std::cout << "Wraps free function : " << (f ? "yes" : "no") << "\n";
    std::cout << "f(3, 4)             : " << f(3, 4) << "\n\n";

    // Demonstrates storing a small lambda inline.
    setTitle("Lambda SBO");
    int bias = 10;
    Function<int(int)> sbo([bias](int x) { return x + bias; });
    std::cout << "Stored inline       : " << (sbo ? "yes" : "no") << "\n";
    std::cout << "sbo(5)              : " << sbo(5) << "\n\n";

    // Demonstrates storing a large lambda on the heap.
    setTitle("Lambda Heap");
    struct BigCapture {
        std::byte pad[64] = {};
        int value = 42;
    } big;
    Function<int()> heap([big]() { return big.value; });
    std::cout << "Stored on heap      : " << (heap ? "yes" : "no") << "\n";
    std::cout << "heap()              : " << heap() << "\n\n";

    // Demonstrates copy construction.
    setTitle("Copy");
    Function<int(int, int)> original(free_add);
    Function<int(int, int)> copied(original);
    std::cout << "Original after copy : " << (original ? "non-empty" : "empty") << "\n";
    std::cout << "Copied              : " << (copied ? "non-empty" : "empty") << "\n";
    std::cout << "copied(2, 3)        : " << copied(2, 3) << "\n\n";

    // Demonstrates move construction.
    setTitle("Move");
    Function<int(int, int)> source(free_add);
    Function<int(int, int)> moved(std::move(source));
    std::cout << "Source after move   : " << (source ? "non-empty" : "empty") << "\n";
    std::cout << "Moved               : " << (moved ? "non-empty" : "empty") << "\n";
    std::cout << "moved(5, 6)         : " << moved(5, 6) << "\n\n";

    // Demonstrates reset and reassignment.
    setTitle("Reset And Reassign");
    Function<int(int)> g([](int x) { return x * 2; });
    std::cout << "Before reset        : " << (g ? "non-empty" : "empty") << "\n";
    std::cout << "g(4)                : " << g(4) << "\n";
    g.reset();
    std::cout << "After reset         : " << (g ? "non-empty" : "empty") << "\n";
    g = [](int x) { return x * 3; };
    std::cout << "After reassign      : " << (g ? "non-empty" : "empty") << "\n";
    std::cout << "g(4)                : " << g(4) << "\n\n";

    // Demonstrates swapping two callable objects.
    setTitle("Swap");
    Function<int(int, int)> a(free_add);
    Function<int(int, int)> b([](int x, int y) { return x * y; });
    std::cout << "a(3, 4) before swap : " << a(3, 4) << "\n";
    std::cout << "b(3, 4) before swap : " << b(3, 4) << "\n";
    a.swap(b);
    std::cout << "a(3, 4) after swap  : " << a(3, 4) << "\n";
    std::cout << "b(3, 4) after swap  : " << b(3, 4) << "\n";

    borderLine();
    std::cout << "\n";
    return 0;
}