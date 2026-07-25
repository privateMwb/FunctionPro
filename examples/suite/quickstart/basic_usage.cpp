// Basic usage of all three FunctionPro types.
//
// Demonstrates:
// - Constructing and invoking a Function (copyable, owning)
// - Constructing and invoking a MoveOnlyFunction (move-only, owning)
// - Constructing and invoking a FunctionRef (non-owning reference)
// - Checking whether each holds/references a callable
// - Resetting a Function and a MoveOnlyFunction back to empty

#include <support/framework.h>

using namespace FunctionPro;

static void run_examples() {

    // Function is the general-purpose, copyable wrapper — the one to
    // reach for by default. A default-constructed Function holds no
    // callable; calling it would throw std::bad_function_call.
    setTitle("Function: Construction and Invocation");

    Function<int(int, int)> add;
    std::cout << "holds callable: " << static_cast<bool>(add) << "\n";

    add = [](int a, int b) { return a + b; };
    std::cout << "holds callable: " << static_cast<bool>(add) << "\n";
    std::cout << "add(2, 3): " << add(2, 3) << "\n\n";

    // MoveOnlyFunction looks and behaves the same way at the call site,
    // but can only be moved, never copied — the type to reach for when
    // the callable itself isn't copyable (e.g. it owns a unique_ptr).
    setTitle("MoveOnlyFunction: Construction and Invocation");

    MoveOnlyFunction<int(int, int)> multiply;
    std::cout << "holds callable: " << static_cast<bool>(multiply) << "\n";

    multiply = [](int a, int b) { return a * b; };
    std::cout << "holds callable: " << static_cast<bool>(multiply) << "\n";
    std::cout << "multiply(2, 3): " << multiply(2, 3) << "\n\n";

    // FunctionRef doesn't own anything or allocate — it's a lightweight
    // reference to a callable that must outlive it. There's no empty
    // default state to check here the same way: it's constructed
    // directly from the callable it will reference.
    setTitle("FunctionRef: Construction and Invocation");

    auto subtract = [](int a, int b) { return a - b; };
    FunctionRef<int(int, int)> subtractRef(subtract);

    std::cout << "holds callable: " << static_cast<bool>(subtractRef) << "\n";
    std::cout << "subtractRef(5, 3): " << subtractRef(5, 3) << "\n\n";

    // Function and MoveOnlyFunction both support reset(), destroying the
    // stored callable and returning to the empty state.
    setTitle("Resetting");

    add.reset();
    multiply.reset();

    std::cout << "add holds callable after reset()     : " << static_cast<bool>(add) << "\n";
    std::cout << "multiply holds callable after reset(): " << static_cast<bool>(multiply) << "\n";
}

REGISTER_EXAMPLE_SUITE();
