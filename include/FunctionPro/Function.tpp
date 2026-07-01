// ============================================================
// Function.tpp
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

#include <stdexcept>
#include <functional>

namespace FunctionPro {

// ============================================================
//  Section 1 — Constructors & Destructor
// ============================================================

template<typename R, typename... Args>
Function<R(Args...)>::Function(std::nullptr_t) noexcept
    : vtable_(nullptr) {}

#ifndef __clang__ 
template<typename R, typename... Args>
template<typename T>
    requires (!std::same_as<std::decay_t<T>, Function<R(Args...)>>)
          && std::is_invocable_r_v<R, std::decay_t<T>, Args...>
          && std::is_copy_constructible_v<std::decay_t<T>>
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

template<typename R, typename... Args>
Function<R(Args...)>::~Function() {
    reset();
}

template<typename R, typename... Args>
Function<R(Args...)>::Function(const Function<R(Args...)>& other) {

    // Clone the stored callable when one exists.
    if (other.vtable_) {
        vtable_ = other.vtable_;
        vtable_->copy(storage_, other.storage_);
    }
}

template<typename R, typename... Args>
Function<R(Args...)>&
Function<R(Args...)>::operator=(const Function<R(Args...)>& other) {

    if (this != &other) {
        reset();

        // Copy the stored callable from the source.
        if (other.vtable_) {
            vtable_ = other.vtable_;
            vtable_->copy(storage_, other.storage_);
        }
    }

    return *this;
}

template<typename R, typename... Args>
Function<R(Args...)>::Function(Function<R(Args...)>&& other) noexcept {

    // Transfer ownership of the stored callable.
    if (other.vtable_) {
        vtable_ = other.vtable_;
        vtable_->move(storage_, other.storage_);

        other.vtable_ = nullptr;
    }
}

template<typename R, typename... Args>
Function<R(Args...)>&
Function<R(Args...)>::operator=(Function<R(Args...)>&& other) noexcept {

    // Prevent self-assignment.
    if (this != &other) {
        reset();

        // Transfer ownership from the source object.
        if (other.vtable_) {
            vtable_ = other.vtable_;
            vtable_->move(storage_, other.storage_);

            other.vtable_ = nullptr;
        }
    }

    return *this;
}


// ============================================================
//  Section 2 — Invocation
// ============================================================

template<typename R, typename... Args>
R Function<R(Args...)>::operator()(Args... args) const {

    // Calling an empty Function matches std::function semantics.
    if (!vtable_)
        throw std::bad_function_call{};

    return vtable_->invoke(
        const_cast<Detail::CallableStorage&>(storage_),
        std::forward<Args>(args)...
    );
}


// ============================================================
//  Section 3 — State
// ============================================================

template<typename R, typename... Args>
Function<R(Args...)>::operator bool() const noexcept {
    return vtable_ != nullptr;
}

template<typename R, typename... Args>
void Function<R(Args...)>::reset() noexcept {

    // Destroy the stored callable and restore the empty state.
    if (vtable_) {
        vtable_->destroy(storage_);
        vtable_       = nullptr;
        storage_.heap = nullptr;
    }
}

template<typename R, typename... Args>
void Function<R(Args...)>::swap(Function& other) noexcept {
    std::swap(vtable_, other.vtable_);
    std::swap(storage_, other.storage_);
}

// ============================================================
//  Section 4 — Equality
// ============================================================

template<typename R, typename... Args>
bool Function<R(Args...)>::operator==(std::nullptr_t) const noexcept {
    return vtable_ == nullptr;
}

template<typename R, typename... Args>
bool Function<R(Args...)>::operator!=(std::nullptr_t) const noexcept {
    return vtable_ != nullptr;
}

} // namespace FunctionPro