// Storing different kinds of callables, across all three types.
//
// Demonstrates:
// - A captureless lambda, a small capture, and a large capture, in a Function
// - The same shapes stored in a MoveOnlyFunction
// - A callable referenced (not stored) through a FunctionRef
// - A plain function pointer and a callable object, held by Function

#include <array>
#include <memory>
#include <support/framework.h>

using namespace FunctionPro;

// A callable object with its own state and an operator().
struct Multiplier {
    int factor;
    int operator()(int x) const {
        return x * factor;
    }
};

static int square(int x) {
    return x * x;
}

static void run_examples() {

    // A captureless lambda decays to a function pointer under the hood
    // and is always small enough to live inline.
    setTitle("Function: Captureless Lambda");

    Function<int(int)> triple = [](int x) { return x * 3; };

    std::cout << "triple(4): " << triple(4) << "\n\n";

    // Small captures — a handful of ints, small strings, etc. — fit
    // inside CallableStorage's inline buffer, so no allocation happens.
    setTitle("Function: Small Capture (Inline)");

    int offset = 100;
    Function<int(int)> addOffset = [offset](int x) { return x + offset; };

    std::cout << "addOffset(5): " << addOffset(5) << "\n\n";

    // Captures too large for the inline buffer are transparently moved
    // to the heap. The call syntax is identical either way.
    setTitle("Function: Large Capture (Heap)");

    std::array<int, 16> weights{};
    weights.fill(2);

    Function<int(int)> weightedSum = [weights](int x) {
        int total = 0;
        for (int w : weights) {
            total += w * x;
        }
        return total;
    };

    std::cout << "weightedSum(3): " << weightedSum(3) << "\n\n";

    // A plain function pointer or a hand-written functor object works
    // the same way — anything invocable with a matching signature.
    setTitle("Function: Function Pointer and Callable Object");

    Function<int(int)> squareFn = &square;
    Function<int(int)> timesFive = Multiplier{5};

    std::cout << "squareFn(6) : " << squareFn(6) << "\n";
    std::cout << "timesFive(7): " << timesFive(7) << "\n\n";

    // MoveOnlyFunction stores callables the exact same way — inline or
    // on the heap, depending on size — the only difference is that it
    // can't be copied afterward.
    setTitle("MoveOnlyFunction: Small and Large Captures");

    int base = 10;
    MoveOnlyFunction<int(int)> addBase = [base](int x) { return x + base; };

    auto resource = std::make_unique<int>(7);
    MoveOnlyFunction<int(int)> addOwned = [resource = std::move(resource)](int x) {
        return x + *resource;
    };

    std::cout << "addBase(5) : " << addBase(5) << "\n";
    std::cout << "addOwned(5): " << addOwned(5) << "\n\n";

    // FunctionRef never stores anything, regardless of the referenced
    // callable's size — it holds a pointer back to the original object,
    // which must keep living for as long as the reference is used.
    setTitle("FunctionRef: Referencing, Not Storing");

    auto halve = [](int x) { return x / 2; };
    FunctionRef<int(int)> halveRef(halve);

    std::cout << "halveRef(10): " << halveRef(10) << "\n";
}

REGISTER_EXAMPLE_SUITE();
