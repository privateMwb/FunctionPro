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
    
    // Clang rejects the constrained out-of-line definition for this constructor,
    // so it is defined inline in the header when compiling with Clang.
    #ifndef __clang__ 
    template<typename T>
        requires (!std::same_as<std::decay_t<T>, FunctionRef<R(Args...)>>)
              && std::is_invocable_r_v<R, T&, Args...>
    FunctionRef(T& callable) noexcept;
    #else
    template<typename T>
        requires (!std::same_as<std::decay_t<T>, FunctionRef<R(Args...)>>)
    && std::is_invocable_r_v<R, T&, Args...>
        FunctionRef(T& callable) noexcept {
        using DecayT = std::decay_t<T>;

        if constexpr (std::is_pointer_v<DecayT> &&
            std::is_function_v<std::remove_pointer_t<DecayT>>) {
            // T is already a function pointer (e.g. int(*)(int,int))
            ptr_.fn = reinterpret_cast<void(*)()>(callable);
            invoke_ = [](PtrStorage p, Args&&... args) -> R {
                return reinterpret_cast<DecayT>(p.fn)(std::forward<Args>(args)...);
                };
        }
        else if constexpr (std::is_function_v<std::remove_reference_t<T>>) {
            // T is a raw function type (e.g. int(int,int)) — decay to pointer first
            ptr_.fn = reinterpret_cast<void(*)()>(static_cast<DecayT>(callable));
            invoke_ = [](PtrStorage p, Args&&... args) -> R {
                return reinterpret_cast<DecayT>(p.fn)(std::forward<Args>(args)...);
                };
        }
        else {
            // T is a callable object — store address through obj
            ptr_.obj = const_cast<void*>(static_cast<const void*>(&callable));
            invoke_ = [](PtrStorage p, Args&&... args) -> R {
                return (*static_cast<DecayT*>(p.obj))(std::forward<Args>(args)...);
                };
        }
    }
    #endif
    
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