// Calling a type that holds no callable, across all three FunctionPro types.
//
// Demonstrates:
// - operator() throwing std::bad_function_call on an empty Function
// - The same mistake with a default-constructed and a reset() Function
// - The correct pattern: check before calling, or catch the exception
// - The identical behavior on an empty MoveOnlyFunction
// - The identical behavior on a default-constructed FunctionRef

#include <stdexcept>
#include <support/framework.h>

using namespace FunctionPro;

static void run_examples() {

    // A default-constructed Function holds no callable. Calling it isn't
    // undefined behavior — it's a well-defined exception, so this is
    // safe to actually run.
    setTitle("Function: Calling a Default-Constructed Function");

    Function<int()> empty;

    try {
        empty();
    } catch (const std::bad_function_call& e) {
        std::cout << "caught: " << e.what() << "\n\n";
    }

    // The same happens after reset(), even if the Function held a
    // callable a moment earlier.
    setTitle("Function: Calling After reset()");

    Function<int()> answer = [] { return 42; };
    answer.reset();

    try {
        answer();
    } catch (const std::bad_function_call& e) {
        std::cout << "caught: " << e.what() << "\n\n";
    }

    // The correct pattern: check operator bool() before calling, so the
    // exception path is never reached in the first place.
    setTitle("Function: The Correct Pattern — Check First");

    Function<int()> maybeSet;

    if (maybeSet) {
        std::cout << "result: " << maybeSet() << "\n";
    } else {
        std::cout << "maybeSet is empty, skipping the call\n\n";
    }

    // If a call is unavoidable without a prior check, catching
    // bad_function_call at the call site is the alternative.
    setTitle("Function: The Correct Pattern — Catch It");

    try {
        maybeSet();
    } catch (const std::bad_function_call&) {
        std::cout << "handled: no callable was bound\n\n";
    }

    // MoveOnlyFunction behaves identically: a default-constructed or
    // reset() instance throws the same std::bad_function_call.
    setTitle("MoveOnlyFunction: The Same Exception");

    MoveOnlyFunction<int()> emptyMoveOnly;

    try {
        emptyMoveOnly();
    } catch (const std::bad_function_call& e) {
        std::cout << "caught: " << e.what() << "\n\n";
    }

    // FunctionRef is default-constructible too, referencing nothing —
    // calling it throws the same exception, even though FunctionRef
    // never owns storage at all.
    setTitle("FunctionRef: The Same Exception");

    FunctionRef<int()> emptyRef;

    try {
        emptyRef();
    } catch (const std::bad_function_call& e) {
        std::cout << "caught: " << e.what() << "\n";
    }
}

REGISTER_EXAMPLE_SUITE();
