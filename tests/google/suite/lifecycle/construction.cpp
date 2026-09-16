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
// Verified via a *class-scoped* operator new/delete override on the
// payload types themselves, rather than a global override. Function
// only ever heap-allocates via `new DecayT(...)` / `delete` on the
// exact stored callable type, so giving that type its own member
// operator new/delete counts precisely the allocations we care about
// without touching GoogleTest's, std::vector's, or anything else's
// internal allocations. This also means multiple test files can each
// define their own counted payload type with no ODR/link collision,
// unlike a process-wide global override.

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdlib>
#include <new>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

namespace {

// Mixin providing per-type allocation counting via member operator
// new/delete. Deriving a payload from this scopes the count to just
// that type's allocations -- it never intercepts anything else in the
// process (gtest's internals included).
struct AllocCounting {
    static inline long allocCount = 0;
    static inline long deallocCount = 0;

    static void* operator new(std::size_t sz) {
        ++allocCount;
        void* p = std::malloc(sz);
        if (!p)
            throw std::bad_alloc();
        return p;
    }
    static void operator delete(void* p) noexcept {
        if (p)
            ++deallocCount;
        std::free(p);
    }
};

// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit.
struct LargePayload : AllocCounting {
    std::array<std::byte, 64> padding{};
};

} // namespace

// Verifies binding a small, SBO-fitting callable to Function causes no
// heap allocation.
TEST(Construction, SmallCaptureNoAllocation) {
    int a = 1, b = 2, c = 3;
    long before = LargePayload::allocCount;
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
    long delta = LargePayload::allocCount - before;

    EXPECT_TRUE(invokeOk);
    EXPECT_TRUE(copyInvokeOk);
    EXPECT_TRUE(moveInvokeOk);
    EXPECT_EQ(delta, 0);
}

// Verifies binding a large, heap-forcing callable to Function causes
// exactly one heap allocation.
TEST(Construction, LargeCaptureAllocatesOnce) {
    LargePayload payload{};
    long before = LargePayload::allocCount;
    bool invokeOk;
    long delta;
    {
        Function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        invokeOk = (f() == 1);
        delta = LargePayload::allocCount - before; // snapshot before exercising copy() below

        // Exercise copy() for this exact lambda binding. This allocates
        // (heap-stored), but it happens after the delta snapshot above so
        // it doesn't affect the "exactly one allocation" assertion.
        Function<int()> g(f);
        EXPECT_EQ(g(), 1);

        // Exercise move() too -- a pointer transfer, so no new allocation.
        Function<int()> h(std::move(g));
        EXPECT_EQ(h(), 1);
    }

    EXPECT_TRUE(invokeOk);
    EXPECT_EQ(delta, 1);
}

// Verifies a Function's heap-allocated callable is released exactly
// once when the Function is destroyed.
TEST(Construction, DestructionReleasesHeap) {
    LargePayload payload{};
    long allocBefore = LargePayload::allocCount;
    long deallocBefore = LargePayload::deallocCount;
    {
        Function<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        EXPECT_EQ(f(), 1); // exercise invoke() before destruction

        {
            Function<int()> g(f); // exercise copy() before destruction
            EXPECT_EQ(g(), 1);

            Function<int()> h(std::move(g)); // exercise move() before destruction
            EXPECT_EQ(h(), 1);
        } // g (moved-from) and h destroyed here -- their heap storage should be released too
    } // f destroyed here -- heap storage should be released
    long allocDelta = LargePayload::allocCount - allocBefore;
    long deallocDelta = LargePayload::deallocCount - deallocBefore;

    EXPECT_EQ(allocDelta, 2);   // one heap allocation for f, one for its copy g
    EXPECT_EQ(deallocDelta, 2); // both released when their scopes end
}

// Verifies binding a small, SBO-fitting callable to MoveOnlyFunction
// causes no heap allocation.
TEST(Construction, MoveOnlySmallNoAlloc) {
    int a = 1, b = 2, c = 3;
    long before = LargePayload::allocCount;
    bool invokeOk, moveInvokeOk;
    {
        MoveOnlyFunction<int()> f = [a, b, c] { return a + b + c; };
        invokeOk = (f() == 6);

        // SBO move doesn't heap-allocate either, so this stays inside the
        // same measured window without affecting delta.
        MoveOnlyFunction<int()> g(std::move(f));
        moveInvokeOk = (g() == 6);
    }
    long delta = LargePayload::allocCount - before;

    EXPECT_TRUE(invokeOk);
    EXPECT_TRUE(moveInvokeOk);
    EXPECT_EQ(delta, 0);
}

// Verifies binding a large, heap-forcing callable to MoveOnlyFunction
// causes exactly one heap allocation.
TEST(Construction, MoveOnlyLargeAllocatesOnce) {
    LargePayload payload{};
    long before = LargePayload::allocCount;
    bool invokeOk;
    long delta;
    {
        MoveOnlyFunction<int()> f = [payload] {
            (void)payload;
            return 1;
        };
        invokeOk = (f() == 1);
        delta = LargePayload::allocCount - before; // snapshot before exercising move() below

        // Exercise move() for this exact lambda binding -- a pointer
        // transfer, so no new allocation after the snapshot above.
        MoveOnlyFunction<int()> g(std::move(f));
        EXPECT_EQ(g(), 1);
    }

    EXPECT_TRUE(invokeOk);
    EXPECT_EQ(delta, 1);
}

// Verifies binding a callable to FunctionRef never allocates, whether
// the referenced callable is small or large -- it only stores an
// address.
TEST(Construction, RefNeverAllocates) {
    int a = 1, b = 2, c = 3;
    LargePayload payload{};

    long before = LargePayload::allocCount;
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
    long delta = LargePayload::allocCount - before;

    EXPECT_TRUE(smallOk);
    EXPECT_TRUE(largeOk);
    EXPECT_EQ(delta, 0);
}
