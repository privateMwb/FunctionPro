// Using a value after it has been moved from, across all three types.
//
// Demonstrates:
// - A moved-from Function is left empty, not in some unspecified state
// - Calling it afterward throws std::bad_function_call rather than
//   invoking stale storage
// - The correct pattern: treat the moved-from variable as done, or
//   reassign it before using it again
// - MoveOnlyFunction behaving identically after a move
// - FunctionRef *not* emptying after a move, since moving it is just a
//   trivial copy of the reference itself

#include <stdexcept>
#include <support/framework.h>

using namespace FunctionPro;

static void run_examples() {

    // Moving constructs the destination from the source's stored
    // callable and leaves the source empty — this is well-defined, so
    // it's safe to actually run and observe.
    setTitle("Function: Moving Leaves the Source Empty");

    Function<int()> original = [] { return 7; };
    Function<int()> moved = std::move(original);

    std::cout << "original holds callable: " << static_cast<bool>(original) << "\n";
    std::cout << "moved    holds callable: " << static_cast<bool>(moved) << "\n\n";

    // Calling the moved-from Function is the mistake: it looks like
    // `original` should still work, but it no longer holds anything.
    setTitle("Function: Calling the Moved-From Function");

    try {
        original();
    } catch (const std::bad_function_call& e) {
        std::cout << "caught: " << e.what() << "\n\n";
    }

    // The correct pattern: once a Function has been moved from, either
    // stop using that variable, or explicitly give it a new callable
    // before calling it again.
    setTitle("Function: The Correct Pattern — Reassign Before Reuse");

    original = [] { return 99; };

    std::cout << "original() after reassignment: " << original() << "\n\n";

    // MoveOnlyFunction behaves exactly the same way: the source is left
    // empty, and calling it throws rather than reading stale storage.
    setTitle("MoveOnlyFunction: The Same Behavior");

    MoveOnlyFunction<int()> moveOnlyOriginal = [] { return 3; };
    MoveOnlyFunction<int()> moveOnlyMoved = std::move(moveOnlyOriginal);

    try {
        moveOnlyOriginal();
    } catch (const std::bad_function_call& e) {
        std::cout << "caught: " << e.what() << "\n\n";
    }

    // FunctionRef is the odd one out here: it owns nothing, so its move
    // constructor is just a trivial copy of the pointer and invoker.
    // The "moved-from" FunctionRef is left completely unchanged and is
    // still perfectly safe to call — there's no source to empty.
    setTitle("FunctionRef: A Move Doesn't Empty It");

    auto triple = [](int x) { return x * 3; };
    FunctionRef<int(int)> ref(triple);
    FunctionRef<int(int)> movedRef = std::move(ref);

    std::cout << "ref(4)     : " << ref(4) << "\n";
    std::cout << "movedRef(4): " << movedRef(4) << "\n";
}

REGISTER_EXAMPLE_SUITE();
