// ============================================================
// fuzz/fuzz_move_only_function.cpp
//
// Fuzzer for FunctionPro::MoveOnlyFunction<int(int)>.
//
// Unlike fuzz_function.cpp, this can't differential-test against a
// std::function-like reference: MoveOnlyFunction wraps genuinely
// move-only callables, and std::move_only_function requires C++23
// (this project targets C++20). Instead, each callable's internal
// state (a single int behind a std::unique_ptr, so the callable
// itself is non-copyable) is tracked by the harness as ground truth:
// SmallMoveOnly/LargeMoveOnly's operator() always does exactly
// `*state += 1; return *state + x;`, so the harness can predict the
// exact next result and the exact next internal value without
// needing a second implementation to compare against.
//
// SmallMoveOnly fits inside MoveOnlyFunction's inline SBO buffer;
// LargeMoveOnly deliberately doesn't, forcing a heap allocation.
//
// Specifically targets:
//   - SBO vs. heap invocation correctness, including repeated
//     reassignment between the small (inline) and large
//     (heap-allocated) callable
//   - move-assign and swap() between two independently tracked slots,
//     including verifying the moved-from side is left empty (a hard
//     guarantee for MoveOnlyFunction, unlike std::function)
//   - self-move-assign and self-swap safety, checked as a
//     bool()-must-be-unchanged invariant
//   - reset() and the bool()/operator==(nullptr) invariant
//   - invoking an empty MoveOnlyFunction throws std::bad_function_call
//
// Deliberately NOT covered: exception injection (MoveOnlyFunction has
// no copy path to inject a throw into in the first place -- there's
// nothing analogous to Function's strong-exception-guarantee
// copy-assign to stress here).
// ============================================================

#include <FunctionPro/MoveOnlyFunction.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <utility>

using FunctionPro::MoveOnlyFunction;

namespace {

// Fits inside MoveOnlyFunction's inline SBO buffer. Move-only because
// of the std::unique_ptr member; its implicit move constructor is
// noexcept (std::unique_ptr's is), satisfying the library's
// nothrow-move-constructible requirement.
struct SmallMoveOnly {
    std::unique_ptr<int> state;
    explicit SmallMoveOnly(int s) : state(std::make_unique<int>(s)) {}
    int operator()(int x) {
        *state += 1;
        return *state + x;
    }
};

// Deliberately larger than the SBO buffer, forcing heap storage.
struct LargeMoveOnly {
    std::unique_ptr<int> state;
    std::array<std::byte, 64> padding{};
    explicit LargeMoveOnly(int s) : state(std::make_unique<int>(s)) {}
    int operator()(int x) {
        *state += 1;
        return *state + x;
    }
};

// Ground truth for one MoveOnlyFunction instance, tracked by the
// harness instead of a second implementation.
struct Model {
    bool empty = true;
    int value = 0;
};

struct Slot {
    MoveOnlyFunction<int(int)> fn;
    Model model;
};

void checkSelfConsistency(const MoveOnlyFunction<int(int)>& fn) {
    if (static_cast<bool>(fn) == (fn == nullptr))
        std::abort();
}

// Aborts (rather than throwing/returning) on mismatch so libFuzzer
// captures a minimal, precise reproducer for exactly the operation
// that broke invariants.
void checkInvoke(Slot& s, int arg) {
    checkSelfConsistency(s.fn);

    const bool fnEmpty = !static_cast<bool>(s.fn);
    if (fnEmpty != s.model.empty)
        std::abort();

    bool threw = false;
    int result = 0;
    try {
        result = s.fn(arg);
    } catch (const std::bad_function_call&) {
        threw = true;
    }

    if (threw != s.model.empty)
        std::abort();

    if (!threw) {
        s.model.value += 1;
        if (result != s.model.value + arg)
            std::abort();
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size == 0)
        return 0;

    Slot a;
    Slot b;

    for (std::size_t i = 0; i < size; ++i) {
        const std::uint8_t byte = data[i];
        const std::uint8_t op = byte % 13;
        const int seed = static_cast<int>(i);
        const int callArg = static_cast<int>(static_cast<std::int8_t>(byte));

        switch (op) {
        case 0: { // assign small callable to a
            a.fn = SmallMoveOnly(seed);
            a.model = Model{false, seed};
            break;
        }
        case 1: { // assign large callable to a
            a.fn = LargeMoveOnly(seed);
            a.model = Model{false, seed};
            break;
        }
        case 2: { // assign small callable to b
            b.fn = SmallMoveOnly(seed);
            b.model = Model{false, seed};
            break;
        }
        case 3: { // assign large callable to b
            b.fn = LargeMoveOnly(seed);
            b.model = Model{false, seed};
            break;
        }
        case 4: { // move-assign a = std::move(b)
            a.fn = std::move(b.fn);
            a.model = b.model;
            b.model = Model{true, 0};
            // MoveOnlyFunction guarantees the moved-from side is left
            // empty -- unlike std::function, this is a hard contract.
            if (static_cast<bool>(b.fn))
                std::abort();
            break;
        }
        case 5: { // swap(a, b)
            FunctionPro::swap(a.fn, b.fn);
            std::swap(a.model, b.model);
            break;
        }
        case 6: { // reset a
            a.fn.reset();
            a.model = Model{true, 0};
            break;
        }
        case 7: { // reset b
            b.fn.reset();
            b.model = Model{true, 0};
            break;
        }
        case 8: { // invoke + check a
            checkInvoke(a, callArg);
            break;
        }
        case 9: { // invoke + check b
            checkInvoke(b, callArg);
            break;
        }
        case 10: { // self-move-assign a: bool() must be unchanged
            const bool wasNonEmpty = static_cast<bool>(a.fn);
            MoveOnlyFunction<int(int)>& selfRef = a.fn;
            a.fn = std::move(selfRef);
            if (static_cast<bool>(a.fn) != wasNonEmpty)
                std::abort();
            break;
        }
        case 11: { // self-swap a: bool() must be unchanged
            const bool wasNonEmpty = static_cast<bool>(a.fn);
            a.fn.swap(a.fn);
            if (static_cast<bool>(a.fn) != wasNonEmpty)
                std::abort();
            break;
        }
        case 12: { // consistency check only, no mutation
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
