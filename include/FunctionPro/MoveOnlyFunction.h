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

    // Move-only type-erased callable wrapper with Small Buffer Optimization.
    // Supports movable callables and prohibits copying.
    template<typename>
    class MoveOnlyFunction;

    template<typename R, typename... Args>
    class MoveOnlyFunction<R(Args...)> {
    private:

        // Core function state.
        const Detail::VTable<R, Args...>* vtable_ = nullptr;
        Detail::CallableStorage           storage_{};

    public:

        // Constructors and destructor.
        MoveOnlyFunction()               noexcept = default;
        MoveOnlyFunction(std::nullptr_t) noexcept;

        template<typename T>
            requires (!std::same_as<std::decay_t<T>, MoveOnlyFunction<R(Args...)>>)
        && std::is_invocable_r_v<R, std::decay_t<T>, Args...>
            MoveOnlyFunction(T&& callable);

        ~MoveOnlyFunction();

        MoveOnlyFunction(const MoveOnlyFunction&) = delete;
        MoveOnlyFunction& operator=(const MoveOnlyFunction&) = delete;

        MoveOnlyFunction(MoveOnlyFunction&& other)             noexcept;
        MoveOnlyFunction& operator=(MoveOnlyFunction&& other)  noexcept;

        // Invokes the stored callable.
        R operator()(Args... args);

        // State.
        [[nodiscard]] explicit operator bool() const noexcept;
        void reset() noexcept;
        void swap(MoveOnlyFunction& other) noexcept;

        // Equality comparison with nullptr.
        [[nodiscard]] bool operator==(std::nullptr_t) const noexcept;
        [[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
    };

    // Exchanges the contents of two MoveOnlyFunction objects.
    template<typename R, typename... Args>
    void swap(MoveOnlyFunction<R(Args...)>& lhs,
        MoveOnlyFunction<R(Args...)>& rhs) noexcept {
        lhs.swap(rhs);
    }

} // namespace FunctionPro

#include "MoveOnlyFunction.tpp"