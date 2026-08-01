/**
 * @file            FunctionRef.h
 *
 * @date            2026-23-7
 *
 * @version         1.0.0
 *
 * @copyright       Copyright (c) 2026 MWB
 *                  All rights reserved.
 *                  https://github.com/privateMwb/FunctionPro
 *
 * @attention       This source is released under the MIT license
 *                  SPDX-License-Identifier: MIT
 *                  <http://opensource.org/licenses/MIT>
 */

#pragma once

// clang-format off
#include <concepts>                             // std::same_as
#include <type_traits>                          // std::decay_t, std::is_invocable_r_v, std::is_pointer_v, std::is_function_v, std::remove_pointer_t, std::remove_reference_t
#include <utility>                              // std::forward
// clang-format on

namespace FunctionPro {

/**
 * @brief Non-owning type-erased callable reference.
 * @tparam R Return type of the call signature (only visible via the
 * `FunctionRef<R(Args...)>` partial specialization below).
 * @tparam Args Argument types of the call signature.
 * @details Performs no allocations and does not own the referenced
 * callable. The callable must outlive the `FunctionRef` instance.
 * Trivially copyable — a `FunctionRef` is just a pointer plus a function
 * pointer, so it is normally passed and returned by value.
 */
template <typename> class FunctionRef;

/// @brief Partial specialization exposing the callable interface for a
/// concrete `R(Args...)` call signature.
template <typename R, typename... Args> class FunctionRef<R(Args...)> {
  private:
    /// @brief Stores either an object pointer or a function pointer,
    /// whichever the referenced callable is.
    union PtrStorage {
        void* obj;    ///< Address of a referenced callable object.
        void (*fn)(); ///< A referenced free function or function pointer,
                      ///< type-erased.
    };

    // Core function reference state.
    PtrStorage ptr_ = {};
    R (*invoke_)(PtrStorage, Args&&...) = nullptr;

  public:
    // Constructors and destructor.

    /// @brief Constructs an empty `FunctionRef` referencing no callable.
    FunctionRef() noexcept = default;

// Clang rejects the constrained out-of-line definition for this constructor,
// so it is defined inline in the header when compiling with Clang.
#ifndef __clang__
    /**
     * @brief Constructs a `FunctionRef` referencing `callable`.
     * @tparam T Deduced type of the callable argument.
     * @param callable Callable to reference. Must outlive this `FunctionRef`.
     * @details Participates in overload resolution only when `T` decays
     * to something other than `FunctionRef` itself and is invocable as
     * `R(Args...)`. Function pointers and raw function references are
     * stored directly rather than through an object address, avoiding an
     * unnecessary extra indirection on every call.
     */
    template <typename T>
        requires(!std::same_as<std::decay_t<T>, FunctionRef<R(Args...)>>) &&
                std::is_invocable_r_v<R, T&, Args...>
    FunctionRef(T& callable) noexcept;
#else
    template <typename T>
        requires(!std::same_as<std::decay_t<T>, FunctionRef<R(Args...)>>) &&
                std::is_invocable_r_v<R, T&, Args...>
    FunctionRef(T& callable) noexcept {
        using DecayT = std::decay_t<T>;

        if constexpr (std::is_pointer_v<DecayT> &&
                      std::is_function_v<std::remove_pointer_t<DecayT>>) {
            // T is already a function pointer (e.g. int(*)(int,int))
            ptr_.fn = reinterpret_cast<void (*)()>(callable);
            invoke_ = [](PtrStorage p, Args&&... args) -> R {
                return reinterpret_cast<DecayT>(p.fn)(std::forward<Args>(args)...);
            };
        } else if constexpr (std::is_function_v<std::remove_reference_t<T>>) {
            // T is a raw function type (e.g. int(int,int)) — decay to pointer first
            ptr_.fn = reinterpret_cast<void (*)()>(static_cast<DecayT>(callable));
            invoke_ = [](PtrStorage p, Args&&... args) -> R {
                return reinterpret_cast<DecayT>(p.fn)(std::forward<Args>(args)...);
            };
        } else {
            // T is a callable object — store address through obj
            ptr_.obj = const_cast<void*>(static_cast<const void*>(&callable));
            invoke_ = [](PtrStorage p, Args&&... args) -> R {
                return (*static_cast<DecayT*>(p.obj))(std::forward<Args>(args)...);
            };
        }
    }
#endif

    /// @brief Trivial destructor: `FunctionRef` owns nothing.
    ~FunctionRef() = default;
    /// @brief Copies the reference (not the referenced callable).
    FunctionRef(const FunctionRef&) = default;
    /// @brief Copies the reference (not the referenced callable).
    FunctionRef& operator=(const FunctionRef&) = default;
    /// @brief Moves the reference (not the referenced callable).
    FunctionRef(FunctionRef&&) noexcept = default;
    /// @brief Moves the reference (not the referenced callable).
    FunctionRef& operator=(FunctionRef&&) noexcept = default;

    /**
     * @brief Invokes the referenced callable.
     * @param args Arguments forwarded to the referenced callable.
     * @return The callable's result.
     * @throws std::bad_function_call if this `FunctionRef` is empty.
     */
    R operator()(Args... args) const;

    // State.

    /// @brief Returns whether this `FunctionRef` references a callable.
    [[nodiscard]] explicit operator bool() const noexcept;

    /// @brief Returns whether this `FunctionRef` is empty.
    [[nodiscard]] bool operator==(std::nullptr_t) const noexcept;

    /// @brief Returns whether this `FunctionRef` references a callable.
    [[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
};

} // namespace FunctionPro

/// @brief Umbrella alias so this library's types are reachable as
/// `rain::FunctionRef`, alongside every other project library, while its
/// true namespace (and all internal diagnostics) remains `FunctionPro`.
/// Reopens `rain` rather than aliasing it, for the same reason as
/// Function.h. Declared separately here because this header doesn't
/// include (and isn't included by) Function.h or MoveOnlyFunction.h --
/// all three are independent entry points. Notably lighter-weight than
/// the other two (no Detail/CallableStorage etc. dependency), so keeping
/// it standalone also avoids forcing that machinery on someone who only
/// wants a non-owning callable reference.
namespace rain {
using namespace FunctionPro;
}

#include "FunctionRef.tpp"
