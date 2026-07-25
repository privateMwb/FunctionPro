// A FunctionRef outliving the callable it references.
//
// Demonstrates:
// - Why FunctionRef never extends the lifetime of what it references
// - The dangling-reference mistake, shown but never executed
// - The correct pattern: keep the referenced callable alive for as long
//   as the FunctionRef is used
// - Why the same function returns a valid Function and MoveOnlyFunction:
//   both own a copy of the callable instead of just referencing it

#include <support/framework.h>

using namespace FunctionPro;

// Returns a FunctionRef bound to a callable that lives only inside this
// function. The reference outlives the object it points to the moment
// this function returns — this is the mistake, never called below.
[[maybe_unused]] static FunctionRef<int()> makeDanglingRef() {
    int local = 42;
    auto lambda = [local] { return local; };
    return FunctionRef<int()>(lambda); // lambda is destroyed on return
}

// Returns a FunctionRef alongside the object it references, so the
// caller controls both lifetimes and can keep the callable alive.
static int callWithLiveCallable(FunctionRef<int()> ref) {
    return ref();
}

static void run_examples() {

    // FunctionRef performs no allocation and does not own what it
    // references — it is only a pointer plus an invoker, so nothing
    // keeps the referenced callable alive on its behalf.
    setTitle("What Goes Wrong");

    std::cout << "makeDanglingRef() would return a FunctionRef whose\n"
                 "referenced lambda was already destroyed:\n\n"
                 "    FunctionRef<int()> ref = makeDanglingRef();\n"
                 "    ref(); // UB: reads a destroyed lambda's storage\n\n"
                 "This call is never made in this example.\n\n";

    // The correct pattern: the callable is declared in the same scope as
    // its use, so it is guaranteed alive for every call through the ref.
    setTitle("The Correct Pattern");

    int local = 42;
    auto lambda = [local] { return local; };
    FunctionRef<int()> ref(lambda);

    std::cout << "callWithLiveCallable(ref): " << callWithLiveCallable(ref) << "\n\n";

    // As long as the referenced object's scope encloses every use of the
    // FunctionRef, passing it around and calling it repeatedly is safe.
    setTitle("Safe to Reuse Within Scope");

    std::cout << "ref(): " << ref() << "\n";
    std::cout << "ref(): " << ref() << "\n\n";

    // Function and MoveOnlyFunction don't have this problem: they own a
    // copy (or a moved-in instance) of the callable inside their own
    // storage, so a function can safely build one from a purely local
    // lambda and return it — nothing dangles.
    setTitle("Function and MoveOnlyFunction Don't Dangle This Way");

    auto makeOwningFunction = [] {
        int local = 99;
        return Function<int()>([local] { return local; }); // owns its own copy
    };

    Function<int()> owned = makeOwningFunction();

    std::cout << "owned(): " << owned() << "\n";

    // MoveOnlyFunction has the same escape from the problem — it moves
    // the callable into its own storage at construction, so it doesn't
    // matter that the lambda below was local to this function either.
    auto makeOwningMoveOnly = [] {
        int local = 7;
        return MoveOnlyFunction<int()>([local] { return local; });
    };

    MoveOnlyFunction<int()> ownedMoveOnly = makeOwningMoveOnly();

    std::cout << "ownedMoveOnly(): " << ownedMoveOnly() << "\n";
}

REGISTER_EXAMPLE_SUITE();
