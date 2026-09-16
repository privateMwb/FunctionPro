// ============================================================
// fuzz/fuzz_function.cpp
//
// Differential fuzzer for FunctionPro::Function<int(int)>, checked
// against a std::function<int(int)> reference after every invocation
// (not just at the end), so a fuzzer-found failure localizes to the
// exact operation that caused it.
//
// The two callable types below (SmallCallable, LargeCallable) are
// wrapped by *both* Function and std::function -- not reimplemented
// separately for each -- so the harness can never disagree with
// itself about what the "correct" result is: both containers hold a
// copy of the exact same object, and the same call sequence is
// replayed against both.
//
// SmallCallable fits inside Function's inline SBO buffer;
// LargeCallable deliberately doesn't, forcing a heap allocation. Both
// mutate an internal counter on every call (via a non-const
// operator()), which specifically stresses Function::operator()
// invoking a mutable callable through its own const member function --
// exactly the const_cast internal to VTableFactory::invoke.
//
// Specifically targets:
//   - SBO vs. heap invocation correctness (SmallCallable vs.
//     LargeCallable), including repeated transitions between the two
//     via reassignment
//   - copy-assign and move-assign, including cross-slot copies/moves
//     between two independently tracked Function/std::function pairs
//   - swap(), including a same-callable cross-slot swap
//   - self-copy-assign and self-move-assign safety (verified as a
//     pure Function-side invariant: bool() must be unchanged by a
//     self-assignment, since std::function's self-move-assignment
//     behavior isn't guaranteed portable enough to differential-test
//     against)
//   - self-swap safety (same reasoning as self-move-assign)
//   - reset() and the empty-vs-non-empty bool()/operator==(nullptr)
//     invariant, which must always agree with each other
//   - invoking an empty Function throws std::bad_function_call, at
//     exactly the same points std::function would
//
// Deliberately NOT covered yet: MoveOnlyFunction (no std::function-like
// reference to differential-test against without requiring C++23's
// std::move_only_function) and FunctionRef (non-owning, so this
// container-lifecycle-style harness doesn't fit it well). Both are
// natural follow-up harnesses, not a change to this one -- see
// FUZZING.md.
// ============================================================

#include <FunctionPro/Function.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <utility>

using FunctionPro::Function;

namespace {

// Fits inside Function's inline SBO buffer.
struct SmallCallable {
    int state;
    explicit SmallCallable(int s) : state(s) {}
    int operator()(int x) {
        state += 1;
        return state + x;
    }
};

// Deliberately larger than Function's SBO buffer, forcing heap storage.
struct LargeCallable {
    int state;
    std::array<std::byte, 64> padding{};
    explicit LargeCallable(int s) : state(s) {}
    int operator()(int x) {
        state += 1;
        return state + x;
    }
};

// A Function/std::function pair driven by identical operations, so
// their observable behavior must always agree.
struct Pair {
    Function<int(int)> fn;
    std::function<int(int)> ref;
};

// The bool()/operator==(nullptr) relationship must always hold,
// independent of anything std::function does.
void checkSelfConsistency(const Function<int(int)>& fn) {
    if (static_cast<bool>(fn) == (fn == nullptr))
        std::abort();
}

// Aborts (rather than throwing/returning) on mismatch so libFuzzer
// captures a minimal, precise reproducer for exactly the operation
// that broke invariants.
void checkPair(Pair& p, int arg) {
    checkSelfConsistency(p.fn);

    const bool fnEmpty = !static_cast<bool>(p.fn);
    const bool refEmpty = !static_cast<bool>(p.ref);
    if (fnEmpty != refEmpty)
        std::abort();

    bool fnThrew = false;
    bool refThrew = false;
    int fnResult = 0;
    int refResult = 0;

    try {
        fnResult = p.fn(arg);
    } catch (const std::bad_function_call&) {
        fnThrew = true;
    }
    try {
        refResult = p.ref(arg);
    } catch (const std::bad_function_call&) {
        refThrew = true;
    }

    if (fnThrew != refThrew)
        std::abort();
    if (!fnThrew && fnResult != refResult)
        std::abort();
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size == 0)
        return 0;

    Pair a;
    Pair b;

    for (std::size_t i = 0; i < size; ++i) {
        const std::uint8_t byte = data[i];
        const std::uint8_t op = byte % 16;
        const int seed = static_cast<int>(i);
        const int callArg = static_cast<int>(static_cast<std::int8_t>(byte));

        switch (op) {
        case 0: { // assign small callable to a
            SmallCallable c(seed);
            a.fn = c;
            a.ref = c;
            break;
        }
        case 1: { // assign large callable to a
            LargeCallable c(seed);
            a.fn = c;
            a.ref = c;
            break;
        }
        case 2: { // assign small callable to b
            SmallCallable c(seed);
            b.fn = c;
            b.ref = c;
            break;
        }
        case 3: { // assign large callable to b
            LargeCallable c(seed);
            b.fn = c;
            b.ref = c;
            break;
        }
        case 4: { // copy-assign a = b
            a.fn = b.fn;
            a.ref = b.ref;
            break;
        }
        case 5: { // move-assign a = std::move(b)
            a.fn = std::move(b.fn);
            a.ref = std::move(b.ref);
            // FunctionPro guarantees the moved-from side is left
            // empty; std::function only guarantees a valid-but-
            // unspecified state, so only the FunctionPro side of this
            // is checked.
            if (static_cast<bool>(b.fn))
                std::abort();
            break;
        }
        case 6: { // swap(a, b)
            FunctionPro::swap(a.fn, b.fn);
            std::swap(a.ref, b.ref);
            break;
        }
        case 7: { // reset a
            a.fn.reset();
            a.ref = nullptr;
            break;
        }
        case 8: { // reset b
            b.fn.reset();
            b.ref = nullptr;
            break;
        }
        case 9: { // invoke + compare a
            checkPair(a, callArg);
            break;
        }
        case 10: { // invoke + compare b
            checkPair(b, callArg);
            break;
        }
        case 11: { // self-copy-assign a: bool() must be unchanged
            const bool wasNonEmpty = static_cast<bool>(a.fn);
            Function<int(int)>& selfRef = a.fn;
            a.fn = selfRef;
            if (static_cast<bool>(a.fn) != wasNonEmpty)
                std::abort();
            break;
        }
        case 12: { // self-move-assign a: bool() must be unchanged
            const bool wasNonEmpty = static_cast<bool>(a.fn);
            Function<int(int)>& selfRef = a.fn;
            a.fn = std::move(selfRef);
            if (static_cast<bool>(a.fn) != wasNonEmpty)
                std::abort();
            break;
        }
        case 13: { // self-swap a: bool() must be unchanged
            const bool wasNonEmpty = static_cast<bool>(a.fn);
            a.fn.swap(a.fn);
            if (static_cast<bool>(a.fn) != wasNonEmpty)
                std::abort();
            break;
        }
        case 14: { // consistency check only, no mutation
            checkSelfConsistency(a.fn);
            checkSelfConsistency(b.fn);
            break;
        }
        default:
            break;
        }

        checkSelfConsistency(a.fn);
        checkSelfConsistency(b.fn);
    }

    return 0;
}
