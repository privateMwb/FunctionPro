#pragma once

#include <FunctionPro/Detail/CallableStorage.h>
#include <FunctionPro/Detail/SBOTraits.h>
#include <FunctionPro/Detail/VTable.h>
#include <FunctionPro/Detail/VTableFactory.h>

#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>


namespace FunctionPro {

    // Type-erased callable wrapper with Small Buffer Optimization (SBO).
    // Provides std::function-like copy, move, and invocation semantics.
    template<typename>
    class Function;

    template<typename R, typename... Args>
    class Function<R(Args...)> {
    private:

        // Core function state.
        const Detail::VTable<R, Args...>* vtable_ = nullptr;
        Detail::CallableStorage           storage_{};

    public:

        // Constructors and destructor.
        Function()               noexcept = default;
        Function(std::nullptr_t) noexcept;

        template<typename T>
            requires (!std::same_as<std::decay_t<T>, Function<R(Args...)>>)
        && std::is_invocable_r_v<R, std::decay_t<T>, Args...>
            && std::is_copy_constructible_v<std::decay_t<T>>
            Function(T&& callable);

        ~Function();

        Function(const Function& other);
        Function& operator=(const Function& other);

        Function(Function&& other)             noexcept;
        Function& operator=(Function&& other)  noexcept;

        // Invokes the stored callable.
        R operator()(Args... args) const;

        // State.
        [[nodiscard]] explicit operator bool() const noexcept;
        void reset() noexcept;
        void swap(Function& other) noexcept;

        // Equality comparison with nullptr.
        [[nodiscard]] bool operator==(std::nullptr_t) const noexcept;
        [[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
    };

    // Swaps two Function objects.
    template<typename R, typename... Args>
    void swap(Function<R(Args...)>& lhs, Function<R(Args...)>& rhs) noexcept {
        lhs.swap(rhs);
    }

} // namespace FunctionPro

#include "Function.tpp"