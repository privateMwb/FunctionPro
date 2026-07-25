// Trying to copy a MoveOnlyFunction.
//
// Demonstrates:
// - Why MoveOnlyFunction's copy constructor and copy assignment are
//   deleted, shown but never compiled
// - The correct pattern: move it instead, or use Function if the
//   callable is actually copyable
// - FunctionRef sidestepping the question entirely: it's always
//   trivially copyable, since copying it never touches the referenced
//   callable

#include <support/framework.h>

using namespace FunctionPro;

static void run_examples() {

    // MoveOnlyFunction deletes both the copy constructor and copy
    // assignment operator. Attempting either is a compile error, not a
    // runtime one — this block is never compiled, only shown.
    setTitle("What Doesn't Compile");

    std::cout << "MoveOnlyFunction<void()> a = [] {};\n"
                 "MoveOnlyFunction<void()> b = a;   // error: copy constructor is deleted\n"
                 "MoveOnlyFunction<void()> c;\n"
                 "c = a;                            // error: copy assignment is deleted\n\n";

    // The correct pattern, if the intent was to transfer ownership:
    // move it. The source is left empty, exactly as with Function.
    setTitle("The Correct Pattern: Move It");

    MoveOnlyFunction<int()> source = [] { return 5; };
    MoveOnlyFunction<int()> destination = std::move(source);

    std::cout << "source holds callable     : " << static_cast<bool>(source) << "\n";
    std::cout << "destination holds callable: " << static_cast<bool>(destination) << "\n";
    std::cout << "destination(): " << destination() << "\n\n";

    // The other correct pattern: if the callable is actually copyable
    // and copies are genuinely needed, use Function instead — that's
    // exactly the distinction between the two types.
    setTitle("The Correct Pattern: Use Function Instead");

    Function<int()> copyable = [] { return 11; };
    Function<int()> copy = copyable; // fine: Function supports copying

    std::cout << "copyable(): " << copyable() << "\n";
    std::cout << "copy()    : " << copy() << "\n\n";

    // A third option: if ownership doesn't need to be transferred at
    // all — the callable will outlive its use — FunctionRef is always
    // copyable, because its copy is just a pointer and an invoker, not
    // the referenced callable itself.
    setTitle("A Third Option: FunctionRef Doesn't Need to Be Copied");

    auto negate = [](int x) { return -x; };
    FunctionRef<int(int)> refA(negate);
    FunctionRef<int(int)> refB = refA; // always fine, regardless of what refA references

    std::cout << "refA(5): " << refA(5) << "\n";
    std::cout << "refB(5): " << refB(5) << "\n";
}

REGISTER_EXAMPLE_SUITE();
