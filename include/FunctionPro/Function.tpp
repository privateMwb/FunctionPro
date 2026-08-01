/**
 * @file Function.tpp
 * @brief Function template implementation.
 *
 * Contains the implementation of FunctionPro::Function's out-of-line
 * member functions.
 */

// ============================================================
// Template implementation for FunctionPro::Function.
// ============================================================
//
//  Sections:
//   1. Constructors & Destructor
//   2. Invocation
//   3. State
//   4. Equality
//
// ============================================================

// clang-format off
#include <functional>                            // std::bad_function_call
#include <stdexcept>                             // (exception base used alongside bad_function_call, if applicable)
// clang-format on

namespace FunctionPro {

// ============================================================
//  Section 1 — Constructors & Destructor
// ============================================================

template <typename R, typename... Args>
Function<R(Args...)>::Function(std::nullptr_t) noexcept : vtable_(nullptr) {}

#ifndef __clang__
template <typename R, typename... Args>
template <typename T>
    requires(!std::same_as<std::decay_t<T>, Function<R(Args...)>>) &&
            std::is_invocable_r_v<R, std::decay_t<T>, Args...> &&
            std::is_copy_constructible_v<std::decay_t<T>>
Function<R(Args...)>::Function(T&& callable) {
    using DecayT = std::decay_t<T>;

    // Store small callables inline; allocate larger ones on the heap.
    if constexpr (Detail::SBOTraits<DecayT>::fits) {
        new (storage_.inlineSlot()) DecayT(std::forward<T>(callable));
    } else {
        *storage_.heapSlot() = new DecayT(std::forward<T>(callable));
    }

    // Bind the callable's type-erased operations.
    vtable_ = Detail::VTableFactory<DecayT, R, Args...>::get();
}
#endif

template <typename R, typename... Args> Function<R(Args...)>::~Function() {
    reset();
}

template <typename R, typename... Args>
Function<R(Args...)>::Function(const Function<R(Args...)>& other) {

    // Clone the stored callable when one exists. `vtable_` is only
    // assigned *after* copy() succeeds: if the callable's copy
    // constructor throws, `storage_` never held a valid object, so
    // vtable_ must remain nullptr -- otherwise ~Function() would later
    // call `destroy` on storage that was never actually constructed.
    if (other.vtable_) {
        other.vtable_->copy(storage_, other.storage_);
        vtable_ = other.vtable_;
    }
}

template <typename R, typename... Args>
Function<R(Args...)>& Function<R(Args...)>::operator=(const Function<R(Args...)>& other) {

    if (this != &other) {
        if (other.vtable_) {
            // Copy into a temporary first. If the callable's copy
            // constructor throws, *this is completely unaffected --
            // matching std::function's strong exception-safety
            // guarantee for copy assignment. Only once the copy has
            // succeeded do we discard our old value and adopt the new
            // one (a noexcept move of the temporary into storage_).
            Detail::CallableStorage tmp;
            other.vtable_->copy(tmp, other.storage_);

            reset();
            other.vtable_->move(storage_, tmp);
            vtable_ = other.vtable_;
        } else {
            reset();
        }
    }

    return *this;
}

template <typename R, typename... Args>
Function<R(Args...)>::Function(Function<R(Args...)>&& other) noexcept {

    // Transfer ownership of the stored callable.
    if (other.vtable_) {
        other.vtable_->move(storage_, other.storage_);
        vtable_ = other.vtable_;
        other.vtable_ = nullptr;
    }
}

template <typename R, typename... Args>
Function<R(Args...)>& Function<R(Args...)>::operator=(Function<R(Args...)>&& other) noexcept {

    // Prevent self-assignment.
    if (this != &other) {
        reset();

        // Transfer ownership from the source object.
        if (other.vtable_) {
            other.vtable_->move(storage_, other.storage_);
            vtable_ = other.vtable_;
            other.vtable_ = nullptr;
        }
    }

    return *this;
}

// ============================================================
//  Section 2 — Invocation
// ============================================================

template <typename R, typename... Args> R Function<R(Args...)>::operator()(Args... args) const {

    // Calling an empty Function matches std::function semantics.
    if (!vtable_)
        throw std::bad_function_call{};

    return vtable_->invoke(const_cast<Detail::CallableStorage&>(storage_),
                           std::forward<Args>(args)...);
}

// ============================================================
//  Section 3 — State
// ============================================================

template <typename R, typename... Args> Function<R(Args...)>::operator bool() const noexcept {
    return vtable_ != nullptr;
}

template <typename R, typename... Args> void Function<R(Args...)>::reset() noexcept {

    // Destroy the stored callable and restore the empty state.
    if (vtable_) {
        vtable_->destroy(storage_);
        vtable_ = nullptr;
    }
}

template <typename R, typename... Args> void Function<R(Args...)>::swap(Function& other) noexcept {

    if (this == &other)
        return;

    auto* const thisVt = vtable_;
    auto* const otherVt = other.vtable_;

    // A raw byte-swap of `storage_` is unsafe in general: an
    // inline-stored (SBO) callable may contain a subobject with an
    // internal pointer back into its own storage (e.g. a captured
    // std::string using short-string optimization on some standard
    // library implementations). Bitwise-swapping such an object's bytes
    // leaves its internal pointer referring to the wrong Function's
    // storage, corrupting it. Swapping through the type's real move
    // constructor (via the vtable) is always safe and is still cheap --
    // for heap-stored callables `move` is just a pointer exchange, and
    // for inline callables it's a placement move-construct plus
    // destroy, exactly as it would be to move the value through a local
    // variable by hand.
    if (thisVt && otherVt) {
        Detail::CallableStorage tmp;
        thisVt->move(tmp, storage_);
        otherVt->move(storage_, other.storage_);
        thisVt->move(other.storage_, tmp);
    } else if (thisVt) {
        thisVt->move(other.storage_, storage_);
    } else if (otherVt) {
        otherVt->move(storage_, other.storage_);
    }

    vtable_ = otherVt;
    other.vtable_ = thisVt;
}

// ============================================================
//  Section 4 — Equality
// ============================================================

template <typename R, typename... Args>
bool Function<R(Args...)>::operator==(std::nullptr_t) const noexcept {
    return vtable_ == nullptr;
}

template <typename R, typename... Args>
bool Function<R(Args...)>::operator!=(std::nullptr_t) const noexcept {
    return vtable_ != nullptr;
}

} // namespace FunctionPro
