// FunctionPro construction test suite.
//
// Coverage:
// - A callable that fits inline (SBO_SIZE = 40 bytes on a 64-bit build)
//   causes zero heap allocations when bound to Function/MoveOnlyFunction
// - A callable too large for SBO causes exactly one heap allocation
// - That heap allocation is released exactly once on destruction
// - FunctionRef never allocates, regardless of the referenced
//   callable's size, since it only ever stores an address
//
// Verified via a global operator new/delete override that counts calls
// -- this is the only way to observe the SBO/heap split from outside
// the class, since it's an implementation detail with no public API to
// query directly. Each check snapshots the allocation count immediately
// before and after the measured construction/destruction, and keeps
// CHK() calls (which may trigger one-time iostream allocations on their
// first-ever use in the process) outside that measurement window.
//
// NOTE: this file defines a process-wide operator new/delete override.
// If the test suite links multiple .cpp files into one binary, this
// will collide (ODR/link error) with any other file doing the same --
// flag it if that happens.

#include <support/framework.h>

#include <array>
#include <cstdlib>
#include <new>

using namespace FunctionPro;

namespace {
long g_allocCount = 0;
long g_deallocCount = 0;

// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit.
struct LargePayload {
    std::array<std::byte, 64> padding{};
};
} // namespace

void* operator new(std::size_t sz) {
    ++g_allocCount;
    void* p = std::malloc(sz);
    if (!p)
        throw std::bad_alloc();
    return p;
}
void operator delete(void* p) noexcept {
    if (p)
        ++g_deallocCount;
    std::free(p);
}
void operator delete(void* p, std::size_t) noexcept {
    if (p)
        ++g_deallocCount;
    std::free(p);
}

// Verifies binding a small, SBO-fitting callable to Function causes no
// heap allocation.
static void function_small_capture_no_allocation() {
    int a = 1, b = 2, c = 3;
    long before = g_allocCount;
    bool invokeOk, copyInvokeOk, moveInvokeOk;
    {
        Function<int()> f = [a, b, c] { return a + b + c; };
        invokeOk = (f() == 6);

        // Exercise copy() for this exact lambda binding. An SBO-fitting
        // callable's copy doesn't heap-allocate either, so this stays
        // inside the same measured window without affecting delta.
        Function<int()> g(f);
        copyInvokeOk = (g() == 6);

        // Same reasoning for move(): SBO move doesn't heap-allocate.
        Function<int()> h(std::move(g));
        moveInvokeOk = (h() == 6);
    }
    long delta = g_allocCount - before;

    CHK(invokeOk);
    CHK(copyInvokeOk);
    CHK(moveInvokeOk);
    CHK(delta == 0);
}

// Verifies binding a large, heap-forcing callable to Function causes
// exactly one heap allocation.
static void function_large_capture_allocates_once() {
    LargePayload payload{};
    long before = g_allocCount;
    bool invokeOk;
    long delta;
    {
        Function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        invokeOk = (f() == 1);
        delta = g_allocCount - before; // snapshot before exercising copy() below

        // Exercise copy() for this exact lambda binding. This allocates
        // (heap-stored), but it happens after the delta snapshot above so
        // it doesn't affect the "exactly one allocation" assertion.
        Function<int()> g(f);
        CHK(g() == 1);

        // Exercise move() too -- a pointer transfer, so no new allocation.
        Function<int()> h(std::move(g));
        CHK(h() == 1);
    }

    CHK(invokeOk);
    CHK(delta == 1);
}

// Verifies a Function's heap-allocated callable is released exactly
// once when the Function is destroyed.
static void function_destruction_releases_heap_allocation() {
    LargePayload payload{};
    long allocBefore = g_allocCount;
    long deallocBefore = g_deallocCount;
    {
        Function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        CHK(f() == 1); // exercise invoke() before destruction

        {
            Function<int()> g(f); // exercise copy() before destruction
            CHK(g() == 1);

            Function<int()> h(std::move(g)); // exercise move() before destruction
            CHK(h() == 1);
        } // g (moved-from) and h destroyed here -- their heap storage should be released too
    } // f destroyed here -- heap storage should be released
    long allocDelta = g_allocCount - allocBefore;
    long deallocDelta = g_deallocCount - deallocBefore;

    CHK(allocDelta == 2);   // one heap allocation for f, one for its copy g
    CHK(deallocDelta == 2); // both released when their scopes end
}

// Verifies binding a small, SBO-fitting callable to MoveOnlyFunction
// causes no heap allocation.
static void move_only_small_capture_no_allocation() {
    int a = 1, b = 2, c = 3;
    long before = g_allocCount;
    bool invokeOk, moveInvokeOk;
    {
        MoveOnlyFunction<int()> f = [a, b, c] { return a + b + c; };
        invokeOk = (f() == 6);

        // SBO move doesn't heap-allocate either, so this stays inside the
        // same measured window without affecting delta.
        MoveOnlyFunction<int()> g(std::move(f));
        moveInvokeOk = (g() == 6);
    }
    long delta = g_allocCount - before;

    CHK(invokeOk);
    CHK(moveInvokeOk);
    CHK(delta == 0);
}

// Verifies binding a large, heap-forcing callable to MoveOnlyFunction
// causes exactly one heap allocation.
static void move_only_large_capture_allocates_once() {
    LargePayload payload{};
    long before = g_allocCount;
    bool invokeOk;
    long delta;
    {
        MoveOnlyFunction<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        invokeOk = (f() == 1);
        delta = g_allocCount - before; // snapshot before exercising move() below

        // Exercise move() for this exact lambda binding -- a pointer
        // transfer, so no new allocation after the snapshot above.
        MoveOnlyFunction<int()> g(std::move(f));
        CHK(g() == 1);
    }

    CHK(invokeOk);
    CHK(delta == 1);
}

// Verifies binding a callable to FunctionRef never allocates, whether
// the referenced callable is small or large -- it only stores an
// address.
static void function_ref_never_allocates() {
    int a = 1, b = 2, c = 3;
    LargePayload payload{};

    long before = g_allocCount;
    bool smallOk, largeOk;
    {
        auto smallCallable = [a, b, c] { return a + b + c; };
        auto largeCallable = [payload] {
            (void)payload;
            return 1;
        };
        FunctionRef<int()> smallRef(smallCallable);
        FunctionRef<int()> largeRef(largeCallable);
        smallOk = (smallRef() == 6);
        largeOk = (largeRef() == 1);
    }
    long delta = g_allocCount - before;

    CHK(smallOk);
    CHK(largeOk);
    CHK(delta == 0);
}

// Executes all construction test cases.
static void run_tests() {
    RUN(function_small_capture_no_allocation);
    RUN(function_large_capture_allocates_once);
    RUN(function_destruction_releases_heap_allocation);

    RUN(move_only_small_capture_no_allocation);
    RUN(move_only_large_capture_allocates_once);

    RUN(function_ref_never_allocates);
}

REGISTER_TEST_SUITE();
