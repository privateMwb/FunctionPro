/**
 * @file            MoveOnlyFunction.h
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
#include <FunctionPro/Detail/CallableStorage.h> // CallableStorage — inline/heap storage for the callable
#include <FunctionPro/Detail/SBOTraits.h>       // SBOTraits<T>::fits — decides inline vs heap storage
#include <FunctionPro/Detail/VTable.h>          // VTable — the type-erased dispatch table this class holds
#include <FunctionPro/Detail/VTableFactory.h>   // VTableFactory — generates the VTable for a bound callable

#include <concepts>                             // std::same_as
#include <cstddef>                              // std::nullptr_t
#include <functional>                           // std::bad_function_call (thrown by operator())
#include <type_traits>                          // std::decay_t, std::is_invocable_r_v
#include <utility>                              // std::forward
// clang-format on

namespace FunctionPro {

/**
 * @brief Move-only type-erased callable wrapper with Small Buffer Optimization.
 * @tparam R Return type of the call signature (only visible via the
 * `MoveOnlyFunction<R(Args...)>` partial specialization below).
 * @tparam Args Argument types of the call signature.
 * @details Supports movable callables and prohibits copying, so it
 * can wrap move-only callables (e.g. one capturing a `std::unique_ptr`)
 * that `Function` cannot. Small callables (see `Detail::SBOTraits`)
 * are stored inline with zero heap allocation; larger callables are
 * allocated on the heap.
 */
template <typename> class MoveOnlyFunction;

/// @brief Partial specialization exposing the callable interface for a
/// concrete `R(Args...)` call signature.
template <typename R, typename... Args> class MoveOnlyFunction<R(Args...)> {
  private:
    // Core function state.
    const Detail::VTable<R, Args...>* vtable_ = nullptr;
    Detail::CallableStorage storage_{};

  public:
    // Constructors and destructor.

    /// @brief Constructs an empty `MoveOnlyFunction` holding no callable.
    MoveOnlyFunction() noexcept = default;

    /// @brief Constructs an empty `MoveOnlyFunction`, equivalent to the default
    /// constructor.
    MoveOnlyFunction(std::nullptr_t) noexcept;

// Clang rejects the constrained out-of-line definition for this constructor,
// so it is defined inline in the header when compiling with Clang.
#ifndef __clang__
    /**
     * @brief Constructs a `MoveOnlyFunction` wrapping `callable`.
     * @tparam T Deduced type of the callable argument.
     * @param callable Callable to store. Stored inline if it fits the
     * SBO buffer (`Detail::SBOTraits<std::decay_t<T>>::fits`), otherwise
     * heap-allocated.
     * @details Participates in overload resolution only when `T` decays
     * to something other than `MoveOnlyFunction` itself and is
     * invocable as `R(Args...)`. Unlike `Function`, copy-constructibility
     * of `T` is not required.
     */
    template <typename T>
        requires(!std::same_as<std::decay_t<T>, MoveOnlyFunction<R(Args...)>>) &&
                std::is_invocable_r_v<R, std::decay_t<T>, Args...>
    MoveOnlyFunction(T&& callable);
#else
    template <typename T>
        requires(!std::same_as<std::decay_t<T>, MoveOnlyFunction<R(Args...)>>) &&
                std::is_invocable_r_v<R, std::decay_t<T>, Args...>
    MoveOnlyFunction(T&& callable) {
        using DecayT = std::decay_t<T>;
        // Store small callables inline; allocate larger ones on the heap.
        if constexpr (Detail::SBOTraits<DecayT>::fits) {
            new (storage_.inlineSlot()) DecayT(std::forward<T>(callable));
        } else {
            *storage_.heapSlot() = new DecayT(std::forward<T>(callable));
        }
        // Bind the callable's move-only type-erased operations.
        vtable_ = Detail::VTableFactory<DecayT, R, Args...>::getMoveOnly();
    }
#endif

    /// @brief Destroys the stored callable, if any.
    ~MoveOnlyFunction();

    /// @brief Deleted: `MoveOnlyFunction` is not copyable.
    MoveOnlyFunction(const MoveOnlyFunction&) = delete;
    /// @brief Deleted: `MoveOnlyFunction` is not copyable.
    MoveOnlyFunction& operator=(const MoveOnlyFunction&) = delete;

    /**
     * @brief Move-constructs from `other`, taking ownership of its stored
     * callable.
     * @param other MoveOnlyFunction to move from. Left empty.
     */
    MoveOnlyFunction(MoveOnlyFunction&& other) noexcept;

    /**
     * @brief Move-assigns from `other`, replacing this MoveOnlyFunction's
     * contents.
     * @param other MoveOnlyFunction to move from. Left empty.
     * @return Reference to `*this`.
     */
    MoveOnlyFunction& operator=(MoveOnlyFunction&& other) noexcept;

    /**
     * @brief Invokes the stored callable.
     * @param args Arguments forwarded to the stored callable.
     * @return The callable's result.
     * @throws std::bad_function_call if this `MoveOnlyFunction` is empty.
     */
    R operator()(Args... args);

    // State.

    /// @brief Returns whether this `MoveOnlyFunction` holds a callable.
    [[nodiscard]] explicit operator bool() const noexcept;

    /// @brief Destroys the stored callable, if any, leaving this
    /// `MoveOnlyFunction` empty.
    void reset() noexcept;

    /**
     * @brief Exchanges the stored callables of `*this` and `other`.
     * @param other MoveOnlyFunction to swap with.
     * @details Always goes through the callable's real move
     * constructor rather than swapping raw storage bytes, so it
     * remains correct even for inline-stored callables containing a
     * subobject with an internal self-pointer.
     */
    void swap(MoveOnlyFunction& other) noexcept;

    /// @brief Returns whether this `MoveOnlyFunction` is empty.
    [[nodiscard]] bool operator==(std::nullptr_t) const noexcept;

    /// @brief Returns whether this `MoveOnlyFunction` holds a callable.
    [[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
};

/**
 * @brief Exchanges the contents of two `MoveOnlyFunction` objects.
 * @tparam R Return type of the call signature.
 * @tparam Args Argument types of the call signature.
 * @param lhs First `MoveOnlyFunction`.
 * @param rhs Second `MoveOnlyFunction`.
 */
template <typename R, typename... Args>
void swap(MoveOnlyFunction<R(Args...)>& lhs, MoveOnlyFunction<R(Args...)>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace FunctionPro

/// @brief Umbrella alias so this library's types are reachable as
/// `rain::MoveOnlyFunction`, alongside every other project library, while
/// its true namespace (and all internal diagnostics) remains `FunctionPro`.
/// Reopens `rain` rather than aliasing it, for the same reason as
/// Function.h. Declared separately here because this header doesn't
/// include (and isn't included by) Function.h or FunctionRef.h -- all
/// three are independent entry points.
namespace rain {
using namespace FunctionPro;
}

#include "MoveOnlyFunction.tpp"
