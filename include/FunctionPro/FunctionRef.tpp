// ============================================================
// FunctionRef.tpp
// Template implementation for FunctionPro::FunctionRef.
// ============================================================
//
//  Sections:
//   1. Constructors
//   2. Invocation
//   3. State
//   4. Equality
//
// ============================================================

#include <functional>

namespace FunctionPro {

    // ============================================================
    //  Section 1 — Constructors
    // ============================================================

    template<typename R, typename... Args>
    template<typename T>
        requires (!std::same_as<std::decay_t<T>, FunctionRef<R(Args...)>>)
    && std::is_invocable_r_v<R, T&, Args...>
        FunctionRef<R(Args...)>::FunctionRef(T& callable) noexcept {
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

    // ============================================================
    //  Section 2 — Invocation
    // ============================================================

    template<typename R, typename... Args>
    R FunctionRef<R(Args...)>::operator()(Args... args) const {

        // Match std::function behavior for empty invocation.
        if (!invoke_)
            throw std::bad_function_call{};

        return invoke_(ptr_, std::forward<Args>(args)...);
    }


    // ============================================================
    //  Section 3 — State
    // ============================================================

    template<typename R, typename... Args>
    FunctionRef<R(Args...)>::operator bool() const noexcept {
        return invoke_ != nullptr;
    }


    // ============================================================
    //  Section 4 — Equality
    // ============================================================

    template<typename R, typename... Args>
    bool FunctionRef<R(Args...)>::operator==(std::nullptr_t) const noexcept {
        return invoke_ == nullptr;
    }

    template<typename R, typename... Args>
    bool FunctionRef<R(Args...)>::operator!=(std::nullptr_t) const noexcept {
        return invoke_ != nullptr;
    }

} // namespace FunctionPro