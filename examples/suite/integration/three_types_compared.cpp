// Choosing between Function, MoveOnlyFunction, and FunctionRef.
//
// Demonstrates:
// - The same lambda bound to all three types, side by side
// - Function's copyability versus MoveOnlyFunction's move-only ownership
// - FunctionRef's zero-allocation, non-owning reference semantics
// - A short guide for picking the right type at a call site

#include <memory>
#include <support/framework.h>

using namespace FunctionPro;

// Accepts a FunctionRef: the cheapest option for a callback that only
// needs to live for the duration of this call.
static int applyRef(FunctionRef<int(int)> fn, int x) {
    return fn(x);
}

static void run_examples() {

    // All three types can wrap the same kind of lambda; the difference
    // is entirely in ownership and copy semantics, not in what they can
    // hold.
    setTitle("The Same Lambda, Three Ways");

    auto triple = [](int x) { return x * 3; };

    Function<int(int)> owningCopyable = triple;
    MoveOnlyFunction<int(int)> owningMoveOnly = triple;
    FunctionRef<int(int)> nonOwning = triple;

    std::cout << "Function(x)         : " << owningCopyable(4) << "\n";
    std::cout << "MoveOnlyFunction(x) : " << owningMoveOnly(4) << "\n";
    std::cout << "FunctionRef(x)      : " << nonOwning(4) << "\n\n";

    // Function can be freely copied — useful when a callback needs to be
    // stored in multiple places at once.
    setTitle("Function: Copyable Ownership");

    Function<int(int)> copyOfCopyable = owningCopyable;

    std::cout << "original(4): " << owningCopyable(4) << "\n";
    std::cout << "copy(4)    : " << copyOfCopyable(4) << "\n\n";

    // MoveOnlyFunction can only be moved, which is what makes it able to
    // hold callables that Function's copy requirement rules out, such as
    // a lambda capturing a unique_ptr.
    setTitle("MoveOnlyFunction: Move-Only Ownership");

    auto resource = std::make_unique<int>(99);
    MoveOnlyFunction<int()> ownsUniquePtr = [resource = std::move(resource)] { return *resource; };
    MoveOnlyFunction<int()> movedOwner = std::move(ownsUniquePtr);

    std::cout << "movedOwner(): " << movedOwner() << "\n\n";

    // FunctionRef performs no allocation and owns nothing — ideal for a
    // parameter that's only used for the duration of one call, like
    // applyRef() below.
    setTitle("FunctionRef: Non-Owning Reference");

    auto square = [](int x) { return x * x; };

    std::cout << "applyRef(square, 5): " << applyRef(square, 5) << "\n";
}

REGISTER_EXAMPLE_SUITE();
