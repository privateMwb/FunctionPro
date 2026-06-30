#pragma once

#include <functional>
#include <type_traits>
#include <utility>

namespace FunctionPro {

// FunctionRef
// non-owning view of a callable — no allocation, no ownership
// the caller must ensure the callable outlives the FunctionRef
template<typename>
class FunctionRef;

template<typename R, typename... Args>
class FunctionRef<R(Args...)> {
private:
    // Core State
    void* ptr_                     = nullptr;
    R (*invoke_)(void*, Args&&...) = nullptr;

public:
    // Constructors & Destructor
    FunctionRef() noexcept = default;

    template<typename T,
             typename = std::enable_if_t<
                 !std::is_same_v<std::decay_t<T>, FunctionRef> &&
                 std::is_invocable_r_v<R, std::decay_t<T>, Args...>>>
    FunctionRef(T&& callable) noexcept;

    ~FunctionRef() = default;

    FunctionRef(const FunctionRef&)            = default;
    FunctionRef& operator=(const FunctionRef&) = default;

    FunctionRef(FunctionRef&&)            noexcept = default;
    FunctionRef& operator=(FunctionRef&&) noexcept = default;

    // Invocation
    R operator()(Args... args) const;

    // State
    [[nodiscard]] explicit operator bool() const noexcept;

    // Equality
    [[nodiscard]] bool operator==(std::nullptr_t) const noexcept;
    [[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
};

} // namespace FunctionPro

#include "FunctionRef.tpp"
