/**
 * @file            Function.h
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
#include <type_traits>                          // std::decay_t, std::is_invocable_r_v, std::is_copy_constructible_v
#include <utility>                              // std::forward, std::move
// clang-format on

namespace FunctionPro {

/**
 * @brief Type-erased callable wrapper with Small Buffer Optimization (SBO).
 * @tparam R Return type of the call signature (only visible via the
 * `Function<R(Args...)>` partial specialization below).
 * @tparam Args Argument types of the call signature.
 * @details Provides `std::function`-like copy, move, and invocation
 * semantics. Small callables (see `Detail::SBOTraits`) are stored
 * inline with zero heap allocation; larger callables are allocated on
 * the heap. Copy assignment provides the strong exception guarantee:
 * if the source callable's copy constructor throws, `*this` is left
 * completely unaffected.
 */
template <typename> class Function;

/// @brief Partial specialization exposing the callable interface for a
/// concrete `R(Args...)` call signature.
template <typename R, typename... Args> class Function<R(Args...)> {
  private:
    // Core function state.
    const Detail::VTable<R, Args...>* vtable_ = nullptr;
    Detail::CallableStorage storage_{};

  public:
    // Constructors and destructor.

    /// @brief Constructs an empty `Function` holding no callable.
    Function() noexcept = default;

    /// @brief Constructs an empty `Function`, equivalent to the default
    /// constructor.
    Function(std::nullptr_t) noexcept;

// Clang rejects the constrained out-of-line definition for this constructor,
// so it is defined inline in the header when compiling with Clang.
#ifndef __clang__
    /**
     * @brief Constructs a `Function` wrapping `callable`.
     * @tparam T Deduced type of the callable argument.
     * @param callable Callable to store. Stored inline if it fits the
     * SBO buffer (`Detail::SBOTraits<std::decay_t<T>>::fits`), otherwise
     * heap-allocated.
     * @details Participates in overload resolution only when `T` decays
     * to something other than `Function` itself, is invocable as
     * `R(Args...)`, and is copy-constructible (required so `Function`
     * itself remains copyable).
     */
    template <typename T>
        requires(!std::same_as<std::decay_t<T>, Function<R(Args...)>>) &&
                std::is_invocable_r_v<R, std::decay_t<T>, Args...> &&
                std::is_copy_constructible_v<std::decay_t<T>>
    Function(T&& callable);
#else
    template <typename T>
        requires(!std::same_as<std::decay_t<T>, Function<R(Args...)>>) &&
                std::is_invocable_r_v<R, std::decay_t<T>, Args...> &&
                std::is_copy_constructible_v<std::decay_t<T>>
    Function(T&& callable) {
        using DecayT = std::decay_t<T>;
        // Store small callables inline; allocate larger ones on the heap.
        if constexpr (Detail::SBOTraits<DecayT>::fits) {
            new (storage_.inlineSlot()) DecayT(std::forward<T>(callable));
        } else {
            *storage_.heapSlot() = new DecayT(std::forward<T>(callable));
        }
        // Bind the callable's type-erased operations.
        vtable_ = Detail::VTableFactory<DecayT, R, Args...>::get();
    }
#endif

    /// @brief Destroys the stored callable, if any.
    ~Function();

    /**
     * @brief Copy-constructs from `other`, cloning its stored callable.
     * @param other Function to copy from.
     * @details Empty if `vtable_` was never assigned, i.e. `other`
     * remains untouched. If the stored callable's copy constructor
     * throws, `*this` is left empty rather than in a broken state.
     */
    Function(const Function& other);

    /**
     * @brief Copy-assigns from `other`, replacing this Function's contents.
     * @param other Function to copy from.
     * @return Reference to `*this`.
     * @details Strong exception guarantee: the source is copied into a
     * temporary first, so if its copy constructor throws, `*this` is
     * completely unaffected — matching `std::function::operator=`.
     */
    Function& operator=(const Function& other);

    /**
     * @brief Move-constructs from `other`, taking ownership of its stored
     * callable.
     * @param other Function to move from. Left empty.
     */
    Function(Function&& other) noexcept;

    /**
     * @brief Move-assigns from `other`, replacing this Function's contents.
     * @param other Function to move from. Left empty.
     * @return Reference to `*this`.
     */
    Function& operator=(Function&& other) noexcept;

    /**
     * @brief Invokes the stored callable.
     * @param args Arguments forwarded to the stored callable.
     * @return The callable's result.
     * @throws std::bad_function_call if this `Function` is empty.
     */
    R operator()(Args... args) const;

    // State.

    /// @brief Returns whether this `Function` holds a callable.
    [[nodiscard]] explicit operator bool() const noexcept;

    /// @brief Destroys the stored callable, if any, leaving this `Function`
    /// empty.
    void reset() noexcept;

    /**
     * @brief Exchanges the stored callables of `*this` and `other`.
     * @param other Function to swap with.
     * @details Always goes through the callable's real move
     * constructor rather than swapping raw storage bytes, so it
     * remains correct even for inline-stored callables containing a
     * subobject with an internal self-pointer (e.g. certain
     * short-string-optimized types).
     */
    void swap(Function& other) noexcept;

    /// @brief Returns whether this `Function` is empty.
    [[nodiscard]] bool operator==(std::nullptr_t) const noexcept;

    /// @brief Returns whether this `Function` holds a callable.
    [[nodiscard]] bool operator!=(std::nullptr_t) const noexcept;
};

/**
 * @brief Swaps two `Function` objects.
 * @tparam R Return type of the call signature.
 * @tparam Args Argument types of the call signature.
 * @param lhs First `Function`.
 * @param rhs Second `Function`.
 */
template <typename R, typename... Args>
void swap(Function<R(Args...)>& lhs, Function<R(Args...)>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace FunctionPro

/// @brief Umbrella alias so this library's types are reachable as
/// `rain::Function`, alongside every other project library, while its true
/// namespace (and all internal diagnostics) remains `FunctionPro`. Reopens
/// `rain` rather than aliasing it, since multiple libraries each contribute
/// their own names into the same `rain` namespace -- an alias
/// (`namespace rain = FunctionPro;`) can only ever bind to one target and
/// collides the moment a second library declares its own `rain` alias to
/// something else. Declared separately in MoveOnlyFunction.h and
/// FunctionRef.h too: none of these three headers includes another, so
/// each is a standalone include point and needs its own declaration.
namespace rain {
using namespace FunctionPro;
}

#include "Function.tpp"
