// Crossing the SBO size threshold, across all three FunctionPro types.
//
// Demonstrates:
// - SBOTraits::fits comparing a callable's size and alignment against
//   CallableStorage's inline capacity
// - A callable that fits inline versus one that doesn't, for Function
// - The same threshold applying identically to MoveOnlyFunction
// - Observing zero heap allocations for the inline case, and one for
//   the heap-allocated case, via an instrumented operator new/delete
// - FunctionRef allocating nothing at all, regardless of the referenced
//   callable's size, since it never stores a copy of it

#include <support/framework.h>

using namespace FunctionPro;

// Counts allocations made through the global operator new, so the
// examples below can observe SBO's effect directly rather than just
// asserting on sizeof.
static std::size_t g_allocations = 0;

void* operator new(std::size_t size) {
    ++g_allocations;
    return std::malloc(size);
}

void operator delete(void* ptr) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
    std::free(ptr);
}

static void run_examples() {

    // SBOTraits<T>::fits compares sizeof(T)/alignof(T) against
    // CallableStorage::SBO_SIZE and SBO_ALIGNMENT — 40 bytes at an
    // 8-byte alignment, as of this storage layout.
    setTitle("The Threshold Itself");

    std::cout << "SBO_SIZE     : " << Detail::CallableStorage::SBO_SIZE << " bytes\n";
    std::cout << "SBO_ALIGNMENT: " << Detail::CallableStorage::SBO_ALIGNMENT << " bytes\n\n";

    // A lambda with a couple of small captures comfortably fits inside
    // the inline buffer — constructing it allocates nothing.
    setTitle("Function: A Callable That Fits Inline");

    std::size_t before = g_allocations;

    int a = 1, b = 2, c = 3;
    auto smallLambda = [a, b, c] { return a + b + c; };

    std::cout << "sizeof(lambda): " << sizeof(smallLambda) << " bytes\n";

    Function<int()> small = smallLambda;

    std::cout << "allocations during construction: " << (g_allocations - before) << "\n";
    std::cout << "small(): " << small() << "\n\n";

    // A lambda capturing enough state to exceed SBO_SIZE no longer
    // fits — VTableFactory falls back to heap storage transparently,
    // and construction now allocates exactly once.
    setTitle("Function: A Callable That Doesn't Fit");

    before = g_allocations;

    std::array<int, 16> bigCapture{};
    bigCapture.fill(4);

    auto largeLambda = [bigCapture] {
        int total = 0;
        for (int v : bigCapture) {
            total += v;
        }
        return total;
    };

    std::cout << "sizeof(lambda): " << sizeof(largeLambda) << " bytes (exceeds SBO_SIZE)\n";

    Function<int()> large = largeLambda;

    std::cout << "allocations during construction: " << (g_allocations - before) << "\n";
    std::cout << "large(): " << large() << "\n\n";

    // The call site never has to know or care which storage strategy is
    // in play — operator() is identical either way.
    setTitle("Function: Same Call Syntax Either Way");

    std::cout << "small(): " << small() << "\n";
    std::cout << "large(): " << large() << "\n\n";

    // MoveOnlyFunction shares the exact same CallableStorage and
    // SBOTraits machinery, so it crosses the same threshold the same
    // way — inline for the small capture, heap for the large one.
    setTitle("MoveOnlyFunction: The Same Threshold");

    before = g_allocations;
    MoveOnlyFunction<int()> smallMoveOnly = smallLambda;
    std::cout << "small capture allocations: " << (g_allocations - before) << "\n";

    before = g_allocations;
    MoveOnlyFunction<int()> largeMoveOnly = largeLambda;
    std::cout << "large capture allocations: " << (g_allocations - before) << "\n\n";

    std::cout << "smallMoveOnly(): " << smallMoveOnly() << "\n";
    std::cout << "largeMoveOnly(): " << largeMoveOnly() << "\n\n";

    // FunctionRef has no SBOTraits check to make, because it never owns
    // storage for the callable at all — it's always just a pointer plus
    // an invoker. Referencing the same oversized lambda that forced
    // Function and MoveOnlyFunction onto the heap allocates nothing.
    setTitle("FunctionRef: No Storage, No Threshold");

    before = g_allocations;

    FunctionRef<int()> largeRef(largeLambda);

    std::cout << "allocations during construction: " << (g_allocations - before) << "\n";
    std::cout << "largeRef(): " << largeRef() << "\n";
}

REGISTER_EXAMPLE_SUITE();
