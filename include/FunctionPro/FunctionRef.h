#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace FunctionPro {

// Non-owning type-erased callable reference.
// Performs no allocations and does not own the referenced callable.
// The callable must outlive the FunctionRef instance.
template<typename>
class FunctionRef;

template<typename R, typename... Args>
class FunctionRef<R(Args...)> {
private:

    // Stores either an object pointer or a function pointer.
    union PtrStorage {
        void*  obj;
        void (*fn)();
    };

    // Core function reference state.
    PtrStorage ptr_ = {};
    R (*invoke_)(PtrStorage, Args&&...) = nullptr;

public:

    // Constructors and destructor.
    FunctionRef() noexcept = default;

    template<typename T>
        requires (!std::same_as<std::decay_t<T>, FunctionRef<R(Args...)>>)
              && std::is_invocable_r_v<R, T&, Args...>
    FunctionRef(T& callable) noexcept;

    ~FunctionRef()                              = default;
    FunctionRef(const FunctionRef&)             = default;
    FunctionRef& operator=(const FunctionRef&)  = default;
    FunctionRef(FunctionRef&&) noexcept         = default;
    FunctionRef& operator=(FunctionRef&&) noexcept = default;

    // Invokes the referenced callable.
    R operator()(Args... args) const;

    // State.
    [[nodiscard]] explicit operator bool() const noexcept;

    // Equality comparison with nullptr.
    [[nodiscard]] bool operator==(std::nullptr_t) const noexcept;
    [[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
};

} // namespace FunctionPro

#include "FunctionRef.tpp"