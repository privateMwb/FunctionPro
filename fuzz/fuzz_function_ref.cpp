// ============================================================
// fuzz/fuzz_function_ref.cpp
//
// Fuzzer for FunctionPro::FunctionRef<int(int)>.
//
// FunctionRef is non-owning and trivially copyable -- there's no
// container lifecycle (no copy/move of *contents*, no heap allocation)
// to fuzz the way fuzz_function.cpp fuzzes Function's storage. Instead
// this repeatedly rebinds a single FunctionRef to a fixed set of
// referents that all outlive the whole run, and after each invocation
// reads the *real* referenced object's own state directly to confirm
// the call actually reached the correct object with the correct
// argument -- rather than trusting FunctionRef's own return value in
// isolation, which would be checking the implementation against
// itself.
//
// Specifically targets:
//   - binding to a mutable functor object (non-const operator())
//   - binding to a `const`-qualified functor object whose operator()
//     is const -- exercising the cv-qualification-preserving invoke
//     path (StoredT = T, not the decayed type) so a const referent is
//     only ever invoked through a const-qualified pointer
//   - binding to a raw function reference (decays to a pointer inside
//     the constructor) and, separately, to an already-a-function-
//     pointer variable -- the two distinct branches in FunctionRef's
//     constructor for free functions
//   - rebinding to empty and confirming invoking an empty FunctionRef
//     throws std::bad_function_call
//   - copying a FunctionRef value and invoking through the copy,
//     confirming it reaches the same underlying object as the
//     original
//   - the bool()/operator==(nullptr) invariant, checked after every
//     single operation
//
// Deliberately NOT covered: referent lifetime violations (invoking a
// FunctionRef after its referent has been destroyed). That's
// documented, contractual UB by design -- "the callable must outlive
// the FunctionRef instance" -- not a bug for this harness to try to
// trigger; every referent here is a stack local that outlives the
// whole fuzz iteration.
// ============================================================

#include <FunctionPro/FunctionRef.h>

#include <cstdint>
#include <cstdlib>
#include <functional>

using FunctionPro::FunctionRef;

namespace {

// Mutable referent: non-const operator(), mutates its own state.
struct MutFunctor {
    int state;
    explicit MutFunctor(int s) : state(s) {}
    int operator()(int x) {
        state += 1;
        return state + x;
    }
};

// Const referent: const operator(), mutates through a mutable member.
// Exercises the cv-qualification-preserving invoke fix -- binding a
// `const ConstFunctor&` must only ever call through a const-qualified
// pointer.
struct ConstFunctor {
    mutable int state;
    explicit ConstFunctor(int s) : state(s) {}
    int operator()(int x) const {
        state += 1;
        return state + x;
    }
};

// Stateless free function referent.
int freeAdd(int x) {
    return x + 1;
}

enum class Target { Empty, Mut, Const, Free };

// Aborts (rather than throwing/returning) on mismatch so libFuzzer
// captures a minimal, precise reproducer for exactly the operation
// that broke invariants.
void invokeAndCheck(FunctionRef<int(int)>& ref, MutFunctor& mutObj, const ConstFunctor& constObj,
                     Target target, int arg) {
    if (static_cast<bool>(ref) == (ref == nullptr))
        std::abort();

    const int mutBefore = mutObj.state;
    const int constBefore = constObj.state;

    bool threw = false;
    int result = 0;
    try {
        result = ref(arg);
    } catch (const std::bad_function_call&) {
        threw = true;
    }

    if (target == Target::Empty) {
        if (!threw)
            std::abort();
        return;
    }
    if (threw)
        std::abort();

    switch (target) {
    case Target::Mut:
        // The real object's own state must have advanced by exactly
        // one -- confirms the call actually reached this object, not
        // some other address.
        if (mutObj.state != mutBefore + 1)
            std::abort();
        if (result != mutObj.state + arg)
            std::abort();
        break;
    case Target::Const:
        if (constObj.state != constBefore + 1)
            std::abort();
        if (result != constObj.state + arg)
            std::abort();
        break;
    case Target::Free:
        if (result != arg + 1)
            std::abort();
        break;
    default:
        break;
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size == 0)
        return 0;

    MutFunctor mutObj(1);
    const ConstFunctor constObj(1);
    int (*fnPtrVar)(int) = freeAdd;

    FunctionRef<int(int)> ref; // starts empty
    Target target = Target::Empty;

    for (std::size_t i = 0; i < size; ++i) {
        const std::uint8_t byte = data[i];
        const std::uint8_t op = byte % 8;
        const int callArg = static_cast<int>(static_cast<std::int8_t>(byte));

        switch (op) {
        case 0: { // bind to the mutable functor (lvalue)
            ref = FunctionRef<int(int)>(mutObj);
            target = Target::Mut;
            break;
        }
        case 1: { // bind to the const functor (lvalue)
            ref = FunctionRef<int(int)>(constObj);
            target = Target::Const;
            break;
        }
        case 2: { // bind to a raw function reference directly
            ref = FunctionRef<int(int)>(freeAdd);
            target = Target::Free;
            break;
        }
        case 3: { // bind to an already-a-function-pointer variable
            ref = FunctionRef<int(int)>(fnPtrVar);
            target = Target::Free;
            break;
        }
        case 4: { // rebind to empty
            ref = FunctionRef<int(int)>();
            target = Target::Empty;
            break;
        }
        case 5: { // copy the FunctionRef value, invoke through the copy
            FunctionRef<int(int)> copy = ref;
            if (static_cast<bool>(copy) != static_cast<bool>(ref))
                std::abort();
            invokeAndCheck(copy, mutObj, constObj, target, callArg);
            break;
        }
        case 6: { // invoke the current ref and check against ground truth
            invokeAndCheck(ref, mutObj, constObj, target, callArg);
            break;
        }
        case 7: { // consistency check only, no mutation
            if (static_cast<bool>(ref) == (ref == nullptr))
                std::abort();
            break;
        }
        default:
            break;
        }

        if (static_cast<bool>(ref) == (ref == nullptr))
            std::abort();
    }

    return 0;
}
