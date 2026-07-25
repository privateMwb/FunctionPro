// Why Function::swap() moves through the real move constructor.
//
// Demonstrates:
// - A callable containing a self-referencing pointer into its own storage
// - swap() producing correct results because it goes through T's move
//   constructor, which fixes up such a pointer
// - What a raw byte-swap of the storage would have broken instead,
//   shown but never executed
// - swap() behaving correctly when only one side holds a callable
// - MoveOnlyFunction::swap() providing the exact same guarantee
// - Why FunctionRef needs no such swap() at all: it never owns storage
//   for the referenced callable to begin with

#include <support/framework.h>

using namespace FunctionPro;

// A minimal stand-in for the kind of type Function::swap() is written to
// protect: an inline buffer plus a pointer that refers back into that
// same buffer. Short-string-optimized strings work this way on some
// standard library implementations.
struct SelfPointing {
    char buffer[8];
    const char* ptr;

    explicit SelfPointing(const char* text) {
        std::snprintf(buffer, sizeof(buffer), "%s", text);
        ptr = buffer; // points into *this*, not into some external object
    }

    // Copy construction must also rebind ptr to this object's buffer.
    SelfPointing(const SelfPointing& other) {
        std::snprintf(buffer, sizeof(buffer), "%s", other.buffer);
        ptr = buffer;
    }

    // The move constructor copies the characters into the new object's
    // own buffer and re-points ptr at that new buffer — it does not
    // just copy the old pointer value.
    SelfPointing(SelfPointing&& other) noexcept {
        std::snprintf(buffer, sizeof(buffer), "%s", other.buffer);
        ptr = buffer;
    }

    const char* operator()() const {
        return ptr;
    }
};

static void run_examples() {

    // Two Functions, each holding a SelfPointing callable whose ptr
    // refers into its own inline storage inside that Function.
    setTitle("Function: Two Self-Referencing Callables");

    Function<const char*()> first = SelfPointing("alpha");
    Function<const char*()> second = SelfPointing("beta");

    std::cout << "first() : " << first() << "\n";
    std::cout << "second(): " << second() << "\n\n";

    // swap() exchanges the two via SelfPointing's real move constructor
    // (through the vtable), so each side's ptr is correctly re-pointed
    // at wherever its bytes actually ended up.
    setTitle("Function: Swapping");

    swap(first, second);

    std::cout << "first() : " << first() << "\n";
    std::cout << "second(): " << second() << "\n\n";

    // A raw byte-swap of the two CallableStorage buffers would instead
    // leave each ptr holding its *old* address — now pointing into the
    // other Function's storage rather than its own. This is never done
    // by FunctionPro and is shown here only for contrast, not executed:
    //
    //     std::byte tmp[sizeof(storage)];
    //     std::memcpy(tmp, &first_storage, sizeof(tmp));
    //     std::memcpy(&first_storage, &second_storage, sizeof(tmp));
    //     std::memcpy(&second_storage, tmp, sizeof(tmp));
    //     // first()'s ptr now dangles into what used to be second's bytes
    setTitle("What a Byte-Swap Would Have Broken");

    std::cout << "(not executed — see comment above)\n\n";

    // swap() also handles the case where only one side holds a callable,
    // transferring it in a single move rather than a three-way exchange.
    setTitle("Function: Swapping With an Empty Function");

    Function<const char*()> empty;

    swap(first, empty);

    std::cout << "first() holds callable: " << static_cast<bool>(first) << "\n";
    std::cout << "empty holds callable   : " << static_cast<bool>(empty) << "\n";
    std::cout << "empty(): " << empty() << "\n\n";

    // MoveOnlyFunction::swap() is implemented the same way, for the same
    // reason — it holds callables inline too, so it needs the same
    // protection against self-referencing subobjects.
    setTitle("MoveOnlyFunction: The Same Guarantee");

    MoveOnlyFunction<const char*()> firstMO = SelfPointing("gamma");
    MoveOnlyFunction<const char*()> secondMO = SelfPointing("delta");

    swap(firstMO, secondMO);

    std::cout << "firstMO() : " << firstMO() << "\n";
    std::cout << "secondMO(): " << secondMO() << "\n\n";

    // FunctionRef doesn't define a swap() at all, and doesn't need one:
    // it never copies SelfPointing's bytes anywhere, so there's no
    // inline storage for a self-pointer to go stale in. Exchanging two
    // FunctionRefs is just exchanging two pointers.
    setTitle("FunctionRef: Nothing to Protect");

    SelfPointing epsilon("epsilon"), zeta("zeta");
    FunctionRef<const char*()> refA(epsilon);
    FunctionRef<const char*()> refB(zeta);

    std::swap(refA, refB);

    std::cout << "refA(): " << refA() << "\n";
    std::cout << "refB(): " << refB() << "\n";
}

REGISTER_EXAMPLE_SUITE();
