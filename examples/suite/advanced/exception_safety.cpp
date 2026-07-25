// The strong exception guarantee on Function's copy assignment.
//
// Demonstrates:
// - A callable whose copy constructor can throw
// - Copy assignment leaving *this completely unaffected when the copy throws
// - Why the copy is built in a temporary before touching *this
// - A successful copy assignment for contrast
// - MoveOnlyFunction sidestepping the whole issue: it only ever moves,
//   so there's no copy constructor to throw in the first place
// - FunctionRef's copy never throws either, because it copies only the
//   reference, never the referenced callable

#include <support/framework.h>

using namespace FunctionPro;

// A callable that throws from its copy constructor once armed, so its
// behavior when copied can be controlled from the example below.
struct ThrowsOnCopy {
    int value;
    bool* armed;

    ThrowsOnCopy(int v, bool* a) : value(v), armed(a) {}

    ThrowsOnCopy(const ThrowsOnCopy& other) : value(other.value), armed(other.armed) {
        if (*armed) {
            throw std::runtime_error("copy constructor failed");
        }
    }

    ThrowsOnCopy(ThrowsOnCopy&&) noexcept = default;

    int operator()() const {
        return value;
    }
};

static void run_examples() {

    // Function::operator=(const Function&) copies the source into a
    // temporary first, and only discards *this's old value once that
    // copy has actually succeeded.
    setTitle("Function: Setting Up a Throwing Copy");

    bool armed = false;
    Function<int()> source = ThrowsOnCopy{42, &armed};
    Function<int()> destination = [] { return -1; };
    armed = true;

    std::cout << "destination() before assignment: " << destination() << "\n\n";

    // With the copy constructor armed to throw, the assignment fails —
    // but *this is left exactly as it was, not half-updated or empty.
    setTitle("Function: Assignment When the Copy Throws");

    try {
        destination = source;
    } catch (const std::exception& e) {
        std::cout << "caught: " << e.what() << "\n";
    }

    std::cout << "destination() after failed assignment: " << destination() << "\n\n";

    // Disarming the callable lets the same assignment succeed. Only now
    // does destination's old callable get replaced.
    setTitle("Function: Assignment When the Copy Succeeds");

    armed = false;
    destination = source;

    std::cout << "destination() after successful assignment: " << destination() << "\n\n";

    // MoveOnlyFunction never has this problem to guard against: it has
    // no copy assignment operator at all, only move assignment, which
    // transfers ThrowsOnCopy's already-constructed state rather than
    // running its (potentially throwing) copy constructor.
    setTitle("MoveOnlyFunction: No Copy Constructor to Throw");

    armed = true;
    MoveOnlyFunction<int()> moveSource = ThrowsOnCopy{7, &armed};
    MoveOnlyFunction<int()> moveDestination = [] { return -1; };

    moveDestination = std::move(moveSource);

    std::cout << "moveDestination(): " << moveDestination() << "\n\n";

    // FunctionRef's copy assignment is trivial (defaulted) — it copies
    // the pointer and invoker, never touching the referenced callable's
    // constructors at all. Even a ThrowsOnCopy referenced this way can
    // never cause a FunctionRef's copy to throw.
    setTitle("FunctionRef: Copy Can't Throw");

    ThrowsOnCopy dangerous{99, &armed};

    FunctionRef<int()> refA(dangerous);
    FunctionRef<int()> refB = refA; // trivial pointer copy, never invokes ThrowsOnCopy's copy ctor

    std::cout << "refA(): " << refA() << "\n";
    std::cout << "refB(): " << refB() << "\n";
}

REGISTER_EXAMPLE_SUITE();
