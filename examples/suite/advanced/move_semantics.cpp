// Move semantics across all three FunctionPro types.
//
// Demonstrates:
// - Move-constructing a Function transfers ownership and empties the source
// - Move-assigning replaces a Function's contents and empties the source
// - MoveOnlyFunction moving a callable that Function's copy requirement
//   would reject outright
// - Moving is a transfer of the callable itself, not a copy of it
// - FunctionRef has nothing to transfer: moving it just moves the
//   reference, leaving the referenced callable exactly where it was

#include <memory>
#include <string>
#include <support/framework.h>

using namespace FunctionPro;

static void run_examples() {

    // Move-construction transfers the stored callable to the new
    // Function and leaves the source holding nothing.
    setTitle("Function: Move-Constructing");

    Function<int()> original = [] { return 10; };
    Function<int()> movedTo = std::move(original);

    std::cout << "original holds callable: " << static_cast<bool>(original) << "\n";
    std::cout << "movedTo  holds callable: " << static_cast<bool>(movedTo) << "\n";
    std::cout << "movedTo(): " << movedTo() << "\n\n";

    // Move-assignment behaves the same way: the destination's previous
    // callable, if any, is discarded, and the source is left empty.
    setTitle("Function: Move-Assigning");

    Function<int()> destination = [] { return 1; };
    destination = std::move(movedTo);

    std::cout << "movedTo     holds callable: " << static_cast<bool>(movedTo) << "\n";
    std::cout << "destination holds callable: " << static_cast<bool>(destination) << "\n";
    std::cout << "destination(): " << destination() << "\n\n";

    // MoveOnlyFunction can hold callables Function cannot, precisely
    // because it never needs a copy constructor — a lambda capturing a
    // unique_ptr is movable but not copyable.
    setTitle("MoveOnlyFunction: Moving a Callable That Can't Be Copied");

    auto resource = std::make_unique<std::string>("owned");
    MoveOnlyFunction<std::string()> holdsUniquePtr = [resource = std::move(resource)] {
        return *resource;
    };

    MoveOnlyFunction<std::string()> newOwner = std::move(holdsUniquePtr);

    std::cout << "holdsUniquePtr holds callable: " << static_cast<bool>(holdsUniquePtr) << "\n";
    std::cout << "newOwner(): " << newOwner() << "\n\n";

    // The moved callable is transferred, not duplicated — there is only
    // ever one live instance of the underlying object across the move.
    // Declaring only a move constructor (no copy constructor) makes
    // this move-only, which is exactly what MoveOnlyFunction is for.
    setTitle("MoveOnlyFunction: A Move Transfers, It Doesn't Duplicate");

    struct NoisyMove {
        int id;
        NoisyMove(int i) : id(i) {
            std::cout << "  constructing #" << id << "\n";
        }
        NoisyMove(NoisyMove&& other) noexcept : id(other.id) {
            std::cout << "  moving #" << id << "\n";
        }
        int operator()() const {
            return id;
        }
    };

    MoveOnlyFunction<int()> first = NoisyMove{7};
    MoveOnlyFunction<int()> second = std::move(first);

    std::cout << "second(): " << second() << "\n\n";

    // FunctionRef owns nothing, so there's no callable for a "move" to
    // transfer. Moving a FunctionRef just moves the reference itself —
    // the pointer and invoker — which is a trivial, cheap operation.
    // The referenced object is completely untouched either way.
    setTitle("FunctionRef: Moving the Reference, Not the Callable");

    auto negate = [](int x) { return -x; };
    FunctionRef<int(int)> ref(negate);
    FunctionRef<int(int)> movedRef = std::move(ref);

    std::cout << "ref(5)     : " << ref(5) << "\n";
    std::cout << "movedRef(5): " << movedRef(5) << "\n";
}

REGISTER_EXAMPLE_SUITE();
