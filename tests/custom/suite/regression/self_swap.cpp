// FunctionPro self-swap regression test suite.
//
// Not a historical bug -- confirmed safe during audit, kept here as a
// guard against a future change removing swap()'s explicit
// `if (this == &other) return;` early-out. Without that guard, the
// three-move rotation (this -> tmp -> other -> this) would use `this`'s
// storage as a move source *after* already having destroyed it in the
// first step, since `this == &other` means storage_ and other.storage_
// are the same memory -- undefined behavior. Stateful captures are used
// here deliberately: a trivial captureless lambda has nothing to
// corrupt either way, so it wouldn't actually exercise the guard.
//
// Coverage:
// - Function::swap(f) called on itself leaves f fully intact, for both
//   SBO-stored (stateful) and heap-stored callables
// - MoveOnlyFunction::swap(f) called on itself leaves f fully intact

#include <support/framework.h>

#include <array>
#include <string>

using namespace FunctionPro;

namespace {
// 64 bytes of capture is comfortably past the 40-byte SBO_SIZE limit.
struct LargePayload {
    std::array<std::byte, 64> padding{};
    int tag;
    int operator()() const {
        return tag;
    }
};

struct StringCallable {
    std::string s;
    std::string operator()() const {
        return s;
    }
};
} // namespace

// Verifies self-swap on a Function holding a stateful SBO-stored
// callable is safe and leaves its content intact.
static void function_self_swap_sbo_is_safe() {
    Function<std::string()> f = StringCallable{"hello self-swap"};

    f.swap(f); // NOLINT(clang-diagnostic-self-move)

    CHK(static_cast<bool>(f));
    CHK(f() == "hello self-swap");
}

// Verifies self-swap on a Function holding a heap-stored callable is
// safe.
static void function_self_swap_heap_is_safe() {
    Function<int()> f = LargePayload{{}, 42};

    f.swap(f); // NOLINT(clang-diagnostic-self-move)

    CHK(static_cast<bool>(f));
    CHK(f() == 42);
}

// Verifies self-swap on a MoveOnlyFunction holding a stateful
// SBO-stored callable is safe and leaves its content intact.
static void move_only_self_swap_sbo_is_safe() {
    MoveOnlyFunction<std::string()> f = StringCallable{"hello self-swap"};

    f.swap(f); // NOLINT(clang-diagnostic-self-move)

    CHK(static_cast<bool>(f));
    CHK(f() == "hello self-swap");
}

// Verifies self-swap on a MoveOnlyFunction holding a heap-stored
// callable is safe.
static void move_only_self_swap_heap_is_safe() {
    MoveOnlyFunction<int()> f = LargePayload{{}, 42};

    f.swap(f); // NOLINT(clang-diagnostic-self-move)

    CHK(static_cast<bool>(f));
    CHK(f() == 42);
}

// Executes all self-swap regression test cases.
static void run_tests() {
    RUN(function_self_swap_sbo_is_safe);
    RUN(function_self_swap_heap_is_safe);

    RUN(move_only_self_swap_sbo_is_safe);
    RUN(move_only_self_swap_heap_is_safe);
}

REGISTER_TEST_SUITE();
