// Building work now, running it later, across all three FunctionPro types.
//
// Demonstrates:
// - Capturing a computation as a Function without running it
// - Passing the deferred work around before invoking it
// - Reusing the same Function across multiple invocations
// - MoveOnlyFunction deferring work that owns a resource
// - FunctionRef deferring a call within the scope that declared it

#include <memory>
#include <string>
#include <support/framework.h>

using namespace FunctionPro;

// Runs whatever work it's handed, whenever the caller decides to.
static int runLater(const Function<int()>& work) {
    return work();
}

static void run_examples() {

    // Wrapping an expression in a Function captures it without evaluating
    // it. Nothing here has run yet.
    setTitle("Function: Capturing Work");

    int base = 10;
    Function<int()> computeArea = [base] { return base * base; };

    std::cout << "computeArea is built, not yet run\n\n";

    // The Function can be handed off — stored, passed by value or
    // reference, returned — and only produces a result once called.
    setTitle("Function: Running It Later");

    std::cout << "result: " << runLater(computeArea) << "\n\n";

    // Because Function is copyable, the same deferred computation can be
    // invoked multiple times without re-specifying it.
    setTitle("Function: Reusing the Same Deferred Work");

    std::cout << "run 1: " << computeArea() << "\n";
    std::cout << "run 2: " << computeArea() << "\n\n";

    // MoveOnlyFunction defers work the same way, but is the right choice
    // when that work owns something that can't be duplicated — here, a
    // unique_ptr captured into the deferred computation itself.
    setTitle("MoveOnlyFunction: Deferring Work That Owns a Resource");

    auto handle = std::make_unique<std::string>("connection");
    MoveOnlyFunction<std::string()> closeLater = [handle = std::move(handle)] {
        return "closed " + *handle;
    };

    std::cout << "closeLater is built, not yet run\n";
    std::cout << "result: " << closeLater() << "\n\n";

    // FunctionRef can defer a call too, as long as the referenced
    // callable and the FunctionRef itself never leave the scope that
    // declared them — here, both live for the rest of run_examples().
    setTitle("FunctionRef: Deferring Within the Same Scope");

    auto computePerimeter = [base] { return base * 4; };
    FunctionRef<int()> perimeterRef(computePerimeter);

    std::cout << "perimeterRef is built, not yet run\n";
    std::cout << "result: " << perimeterRef() << "\n";
}

REGISTER_EXAMPLE_SUITE();
