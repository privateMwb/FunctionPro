// Checking whether each type holds or references a callable.
//
// Demonstrates:
// - operator bool() and == nullptr on a Function
// - The same checks on a MoveOnlyFunction
// - The same checks on a FunctionRef
// - Copying a Function versus the trivial copy of a FunctionRef
// - Swapping two Functions, including the empty/non-empty state itself

#include <support/framework.h>

using namespace FunctionPro;

static void run_examples() {

    // operator bool() is explicit, so it's used directly in conditions
    // rather than implicitly converting in other contexts.
    setTitle("Function: operator bool() and nullptr");

    Function<int(int)> doubler = [](int x) { return x * 2; };
    Function<int(int)> emptyFn;

    std::cout << "doubler holds callable : " << static_cast<bool>(doubler) << "\n";
    std::cout << "emptyFn == nullptr     : " << (emptyFn == nullptr) << "\n";
    std::cout << "doubler != nullptr     : " << (doubler != nullptr) << "\n\n";

    // MoveOnlyFunction exposes the exact same state checks.
    setTitle("MoveOnlyFunction: operator bool() and nullptr");

    MoveOnlyFunction<int(int)> negate = [](int x) { return -x; };
    MoveOnlyFunction<int(int)> emptyMoveOnly;

    std::cout << "negate holds callable      : " << static_cast<bool>(negate) << "\n";
    std::cout << "emptyMoveOnly == nullptr   : " << (emptyMoveOnly == nullptr) << "\n";
    std::cout << "negate != nullptr          : " << (negate != nullptr) << "\n\n";

    // FunctionRef supports the same checks too, though in practice it's
    // usually constructed directly from a live callable rather than
    // left in a default state.
    setTitle("FunctionRef: operator bool() and nullptr");

    auto increment = [](int x) { return x + 1; };
    FunctionRef<int(int)> incrementRef(increment);

    std::cout << "incrementRef holds callable: " << static_cast<bool>(incrementRef) << "\n";
    std::cout << "incrementRef != nullptr    : " << (incrementRef != nullptr) << "\n\n";

    // Copying a Function produces a second, fully independent Function
    // that also holds a callable.
    setTitle("Function: State After Copy");

    Function<int(int)> copy = doubler;

    std::cout << "doubler holds callable: " << static_cast<bool>(doubler) << "\n";
    std::cout << "copy    holds callable: " << static_cast<bool>(copy) << "\n\n";

    // FunctionRef's copy is trivial — it copies the pointer and the
    // invoker, not the referenced callable, so both refer to the same
    // object afterward. (MoveOnlyFunction can't be copied at all; see
    // misuse/copying_move_only.cpp.)
    setTitle("FunctionRef: Trivial Copy");

    FunctionRef<int(int)> incrementRefCopy = incrementRef;

    std::cout << "incrementRef(4)    : " << incrementRef(4) << "\n";
    std::cout << "incrementRefCopy(4): " << incrementRefCopy(4) << "\n\n";

    // swap() exchanges which callable each Function holds, including
    // the empty/non-empty state itself.
    setTitle("Function: State After Swap");

    swap(emptyFn, copy);

    std::cout << "emptyFn holds callable after swap: " << static_cast<bool>(emptyFn) << "\n";
    std::cout << "copy    holds callable after swap: " << static_cast<bool>(copy) << "\n";
}

REGISTER_EXAMPLE_SUITE();
