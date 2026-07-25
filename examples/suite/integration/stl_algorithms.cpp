// Using all three FunctionPro types with the standard algorithms library.
//
// Demonstrates:
// - FunctionRef as a zero-overhead predicate for std::sort
// - FunctionRef as a comparator for std::find_if
// - Function stored and reused as a transform in std::transform
// - MoveOnlyFunction passed by move into std::for_each, which only
//   requires the function object to be move-constructible

#include <algorithm>
#include <memory>
#include <numeric>
#include <string>
#include <support/framework.h>
#include <vector>

using namespace FunctionPro;

static void run_examples() {

    // FunctionRef adds no indirection beyond a plain function pointer, so
    // it's a natural fit for algorithm parameters that only need to live
    // for the duration of one call.
    setTitle("std::sort with a FunctionRef Comparator");

    std::vector<int> values{5, 2, 8, 1, 9, 3};

    auto descending = [](int a, int b) { return a > b; };
    std::sort(values.begin(), values.end(), FunctionRef<bool(int, int)>(descending));

    std::cout << "sorted descending: ";
    for (int v : values) {
        std::cout << v << " ";
    }
    std::cout << "\n\n";

    // The same reference type works for search predicates, referencing
    // the caller's lambda without copying its captured state.
    setTitle("std::find_if with a FunctionRef Predicate");

    int threshold = 5;
    auto exceedsThreshold = [threshold](int v) { return v > threshold; };

    auto it = std::find_if(values.begin(), values.end(), FunctionRef<bool(int)>(exceedsThreshold));

    if (it != values.end()) {
        std::cout << "first value over " << threshold << ": " << *it << "\n\n";
    }

    // Function is a better fit when the transform needs to be stored,
    // reused across multiple algorithm calls, or outlive the call that
    // built it.
    setTitle("std::transform with a Stored Function");

    Function<int(int)> square = [](int v) { return v * v; };

    std::vector<int> squared(values.size());
    std::transform(values.begin(), values.end(), squared.begin(), square);

    std::cout << "squared: ";
    for (int v : squared) {
        std::cout << v << " ";
    }
    std::cout << "\n\n";

    // The same Function can be reused in a second, unrelated algorithm
    // call without rebuilding it.
    setTitle("Reusing the Same Function");

    int sumOfSquares = std::accumulate(values.begin(), values.end(), 0,
                                       [&square](int acc, int v) { return acc + square(v); });

    std::cout << "sum of squares: " << sumOfSquares << "\n\n";

    // MoveOnlyFunction can be used with algorithms too, as long as it's
    // passed by move: std::for_each only requires its function object to
    // be move-constructible, not copyable, so this works even though the
    // callable itself owns a unique_ptr.
    setTitle("MoveOnlyFunction: std::for_each by Move");

    auto label = std::make_unique<std::string>("value:");
    MoveOnlyFunction<void(int)> printLabeled = [label = std::move(label)](int v) {
        std::cout << *label << " " << v << "\n";
    };

    std::for_each(values.begin(), values.end(), std::move(printLabeled));
}

REGISTER_EXAMPLE_SUITE();
