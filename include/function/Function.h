#pragma once

#include "detail/Storage.h"
#include "detail/SBOTraits.h"
#include "detail/VTable.h"
#include "detail/VTableFactory.h"

namespace FunctionPro {

// Function
// copyable callable wrapper with SBO
template<typename>
class Function;

template<typename R, typename... Args>
class Function<R(Args...)> {
private:
    // Core State
    const VTable<R, Args...>* vtable_ = nullptr;
    Storage                   storage_;

public:
    // Constructors & Destructor
    Function()                noexcept = default;
    Function(std::nullptr_t)  noexcept;

    template<typename T,
             typename = std::enable_if_t<
                 !std::is_same_v<std::decay_t<T>, Function> &&
                 std::is_invocable_r_v<R, std::decay_t<T>, Args...>>>
    Function(T&& callable);

    ~Function();

    Function(const Function& other);
    Function& operator=(const Function& other);

    Function(Function&& other)            noexcept;
    Function& operator=(Function&& other) noexcept;

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

#include "Function.tpp"
