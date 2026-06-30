#pragma once

#include "detail/Storage.h"
#include "detail/SBOTraits.h"
#include "detail/VTable.h"
#include "detail/VTableFactory.h"

namespace FunctionPro {

// MoveOnlyFunction
// move-only callable wrapper with SBO
template<typename>
class MoveOnlyFunction;

template<typename R, typename... Args>
class MoveOnlyFunction<R(Args...)> {
private:
    // Core State
    const VTable<R, Args...>* vtable_ = nullptr;
    Storage                   storage_;

public:
    // Constructors & Destructor
    MoveOnlyFunction() noexcept = default;
    MoveOnlyFunction(std::nullptr_t) noexcept;

    template<typename T,
             typename = std::enable_if_t<
                 !std::is_same_v<std::decay_t<T>, MoveOnlyFunction> &&
                 std::is_invocable_r_v<R, std::decay_t<T>, Args...>>>
    MoveOnlyFunction(T&& callable);

    ~MoveOnlyFunction();

    MoveOnlyFunction(const MoveOnlyFunction&)             = delete;
    MoveOnlyFunction& operator=(const MoveOnlyFunction&)  = delete;

    MoveOnlyFunction(MoveOnlyFunction&& other)             noexcept;
    MoveOnlyFunction& operator=(MoveOnlyFunction&& other)  noexcept;

    // Invocation
    R operator()(Args... args) const;

    // State
    [[nodiscard]] explicit operator bool() const noexcept;
    void reset() noexcept;

    // Equality
    [[nodiscard]] bool operator==(std::nullptr_t) const noexcept;
    [[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
};

} // namespace FunctionPro

#include "MoveOnlyFunction.tpp"
