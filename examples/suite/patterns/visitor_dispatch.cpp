// Dispatching on a key through a table of handlers, all three ways.
//
// Demonstrates:
// - A table mapping keys to FunctionRef handlers, built and used within
//   a single scope
// - Dispatching without owning or copying the underlying callables
// - Falling back when a key has no handler
// - A Function-based table that can be returned from a factory and
//   outlive the scope that built it, unlike FunctionRef
// - A MoveOnlyFunction-based table of one-shot handlers, each owning
//   its own state, removed from the table once dispatched

#include <map>
#include <memory>
#include <string>
#include <support/framework.h>

using namespace FunctionPro;

enum class Shape { Circle, Square, Triangle };

static double circleArea(double side) {
    return 3.14159 * side * side;
}

static double squareArea(double side) {
    return side * side;
}

// Returns a table that outlives this function's own stack frame — every
// captured value has to be owned by the table itself, which is exactly
// what Function (not FunctionRef) makes possible here.
static std::map<Shape, Function<double(double)>> buildAreaTable(double scaleFactor) {
    return {
        {Shape::Circle, [scaleFactor](double side) { return scaleFactor * circleArea(side); }},
        {Shape::Square, [scaleFactor](double side) { return scaleFactor * squareArea(side); }},
    };
}

static void run_examples() {

    // FunctionRef is the natural fit for a dispatch table built for the
    // duration of one call: it references existing callables rather than
    // owning copies of them.
    setTitle("FunctionRef: Building the Dispatch Table");

    auto triangleArea = [](double side) { return 0.4330127 * side * side; };

    std::map<Shape, FunctionRef<double(double)>> handlers{
        {Shape::Circle, circleArea},
        {Shape::Square, squareArea},
        {Shape::Triangle, triangleArea},
    };

    std::cout << "handlers registered: " << handlers.size() << "\n\n";

    // Dispatching looks up the key, then calls through the referenced
    // handler exactly as if it had been called directly.
    setTitle("FunctionRef: Dispatching by Key");

    for (auto shape : {Shape::Circle, Shape::Square, Shape::Triangle}) {
        std::cout << "area: " << handlers.at(shape)(2.0) << "\n";
    }

    std::cout << "\n";

    // A missing key has no handler to dispatch to — check before looking
    // one up rather than risking an out-of-range access.
    setTitle("FunctionRef: Missing Handler");

    Shape unknown = static_cast<Shape>(99);

    if (handlers.find(unknown) == handlers.end()) {
        std::cout << "no handler registered for this shape\n\n";
    }

    // A Function-based table can be built inside one function and
    // returned to the caller — every handler owns its captured state,
    // so nothing dangles once buildAreaTable()'s stack frame is gone.
    setTitle("Function: A Table That Outlives Its Builder");

    auto scaledHandlers = buildAreaTable(2.0);

    std::cout << "scaled area: " << scaledHandlers.at(Shape::Circle)(2.0) << "\n\n";

    // MoveOnlyFunction suits a table of one-shot handlers — each one
    // owns a resource, runs exactly once, and is erased from the table
    // immediately after, rather than staying around to be called again.
    setTitle("MoveOnlyFunction: One-Shot Handlers");

    std::map<Shape, MoveOnlyFunction<void()>> oneShotHandlers;

    auto label = std::make_unique<std::string>("circle initialized");
    oneShotHandlers.emplace(Shape::Circle,
                            [label = std::move(label)] { std::cout << *label << "\n"; });

    auto it = oneShotHandlers.find(Shape::Circle);
    if (it != oneShotHandlers.end()) {
        it->second();
        oneShotHandlers.erase(it);
    }

    std::cout << "handlers remaining: " << oneShotHandlers.size() << "\n";
}

REGISTER_EXAMPLE_SUITE();
