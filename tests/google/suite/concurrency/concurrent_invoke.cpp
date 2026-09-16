// FunctionPro concurrent invoke test suite.
//
// Coverage:
// - Multiple threads invoking the SAME shared Function object
//   concurrently is race-free: operator() is const and only reads
//   storage_/vtable_ (the vtable itself is a compile-time-constructed
//   `constexpr` object, not a runtime lazy-init -- confirmed by reading
//   VTableFactory::get(), so there's no first-call initialization race
//   to worry about either)
// - Multiple threads each owning a separate Function instance and
//   invoking concurrently is race-free (no shared mutable state at all)
// - MoveOnlyFunction's operator() is non-const, so sharing a single
//   instance across threads is not a documented-safe pattern -- only
//   the separate-instance-per-thread case is covered for it, matching
//   the library's actual contract (no internal synchronization; safety
//   depends on the caller's access pattern)
// - FunctionRef: multiple threads invoking through a single shared
//   FunctionRef (itself referencing one shared external callable) is
//   race-free, same reasoning as Function's shared case. Multiple
//   threads each constructing their own local FunctionRef bound to
//   that same shared external callable is also race-free -- this is
//   FunctionRef's actual intended usage pattern: cheap, non-owning
//   views handed out per call site rather than one instance passed
//   around.
//
// Build this file with ThreadSanitizer (-fsanitize=thread) to verify;
// these checks pass trivially without it since a data race doesn't
// guarantee an observable wrong answer on every run.

#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include <FunctionPro/Function.h>
#include <FunctionPro/FunctionRef.h>
#include <FunctionPro/MoveOnlyFunction.h>

using namespace FunctionPro;

namespace {
constexpr int kThreadCount = 16;

struct Doubler {
    int operator()(int x) const {
        return x * 2;
    }
};
} // namespace

// Verifies concurrent invocation of a single shared Function through a
// const reference is race-free and produces correct results.
TEST(ConcurrentInvoke, SharedFunctionInvoke) {
    const Function<int(int)> f = [](int x) { return x * 2; };
    std::vector<int> results(kThreadCount, -1);
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&f, &results, i] { results[i] = f(i); });
    }
    for (auto& t : threads)
        t.join();

    for (int i = 0; i < kThreadCount; ++i) {
        EXPECT_EQ(results[i], i * 2);
    }

    Function<int(int)> g(f); // exercise copy() for this binding (f is const)
    EXPECT_EQ(g(21), 42);

    Function<int(int)> h(std::move(g)); // exercise move() for this binding
    EXPECT_EQ(h(21), 42);
}

// Verifies concurrent invocation across separate, independently-owned
// Function instances (one per thread) is race-free.
TEST(ConcurrentInvoke, SeparateFunctionInvoke) {
    std::vector<int> results(kThreadCount, -1);
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&results, i] {
            Function<int()> f = [i] { return i * 3; };

            Function<int()> g(f);            // exercise copy() for this binding
            Function<int()> h(std::move(g)); // exercise move() for this binding

            results[i] = h();
        });
    }
    for (auto& t : threads)
        t.join();

    for (int i = 0; i < kThreadCount; ++i) {
        EXPECT_EQ(results[i], i * 3);
    }
}

// Verifies concurrent invocation across separate, independently-owned
// MoveOnlyFunction instances (one per thread) is race-free.
TEST(ConcurrentInvoke, SeparateMoveOnlyInvoke) {
    std::vector<int> results(kThreadCount, -1);
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&results, i] {
            MoveOnlyFunction<int()> f = [i] { return i * 5; };
            MoveOnlyFunction<int()> g(std::move(f)); // exercise move() for this binding
            results[i] = g();
        });
    }
    for (auto& t : threads)
        t.join();

    for (int i = 0; i < kThreadCount; ++i) {
        EXPECT_EQ(results[i], i * 5);
    }
}

// Verifies concurrent invocation through a single shared FunctionRef,
// itself referencing one shared external callable, is race-free.
TEST(ConcurrentInvoke, SharedRefInvoke) {
    Doubler doubler;
    FunctionRef<int(int)> ref(doubler);

    std::vector<int> results(kThreadCount, -1);
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&ref, &results, i] { results[i] = ref(i); });
    }
    for (auto& t : threads)
        t.join();

    for (int i = 0; i < kThreadCount; ++i) {
        EXPECT_EQ(results[i], i * 2);
    }
}

// Verifies concurrent invocation across separate, per-thread FunctionRef
// instances all bound to the same shared external callable is
// race-free -- the intended usage pattern for a non-owning view type.
TEST(ConcurrentInvoke, SeparateRefsSharedCallable) {
    Doubler doubler; // single shared external callable

    std::vector<int> results(kThreadCount, -1);
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&doubler, &results, i] {
            FunctionRef<int(int)> ref(doubler); // own local view, same referent
            results[i] = ref(i);
        });
    }
    for (auto& t : threads)
        t.join();

    for (int i = 0; i < kThreadCount; ++i) {
        EXPECT_EQ(results[i], i * 2);
    }
}
