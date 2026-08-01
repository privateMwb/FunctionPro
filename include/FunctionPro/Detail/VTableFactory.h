/**
 * @file VTableFactory.h
 * @brief Generates per-callable-type `VTable` operations for FunctionPro.
 *
 * Contains the factory that, for each concrete callable type erased by
 * `Function` or `MoveOnlyFunction`, generates the invoke/copy/move/destroy
 * functions bound into that type's shared `VTable`.
 */

#pragma once

// clang-format off
#include "CallableStorage.h"        // CallableStorage — storage the generated operations act on
#include "SBOTraits.h"              // SBOTraits<T>::fits — compile-time inline-vs-heap dispatch
#include "VTable.h"                 // VTable — operation table this factory populates

#include <utility>                  // std::move, std::forward
// clang-format on

namespace FunctionPro::Detail {

/**
 * @brief Creates the type-erased operation table for a concrete callable type.
 * @tparam T Concrete (decayed) callable type being erased.
 * @tparam R Return type of the call signature.
 * @tparam Args Argument types of the call signature.
 * @details Generates the functions required to invoke, copy, move, and
 * destroy a stored callable while transparently handling both Small
 * Buffer Optimization (SBO) and heap storage.
 *
 * Every operation here is instantiated per concrete callable type `T`,
 * so whether `T` fits inline (`Detail::SBOTraits<T>::fits`) is known at
 * compile time. All dispatch between inline and heap storage is
 * therefore done with `if constexpr`, never a runtime branch — in
 * particular, `invoke()` (the hottest path, executed on every call)
 * never has to test whether the storage is inline or heap-allocated at
 * runtime.
 */
template <typename T, typename R, typename... Args> struct VTableFactory {

    /**
     * @brief Invokes the stored callable.
     * @param storage Storage holding the live `T` instance.
     * @param args Arguments forwarded into the callable.
     * @return The callable's result.
     */
    static R invoke(CallableStorage& storage, Args&&... args) {
        if constexpr (SBOTraits<T>::fits) {
            return (*static_cast<T*>(storage.inlineSlot()))(std::forward<Args>(args)...);
        } else {
            return (*static_cast<T*>(storage.heap))(std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Copy-constructs the callable into the destination storage.
     * @param dst Uninitialized destination storage.
     * @param src Storage holding the live `T` instance to copy.
     * @throws Whatever `T`'s copy constructor throws. `dst` is left
     * untouched if it throws.
     */
    static void copy(CallableStorage& dst, const CallableStorage& src) {
        if constexpr (SBOTraits<T>::fits) {
            new (dst.inlineSlot()) T(*static_cast<const T*>(src.inlineSlot()));
        } else {
            *dst.heapSlot() = new T(*static_cast<const T*>(src.heap));
        }
    }

    /**
     * @brief Move-constructs the callable into the destination storage
     * and ends the source callable's lifetime.
     * @param dst Uninitialized destination storage.
     * @param src Storage holding the live `T` instance to move from. Left
     * fully destroyed (not just moved-from) on return.
     * @details The inline (SBO) path move-constructs into `dst` and then
     * explicitly destroys the source object, so a moved-from callable
     * stored inline is never left un-destroyed — matching how a
     * moved-from object stored in a standard container is still
     * destroyed when the container gives it up. The heap path is a
     * direct pointer transfer: a non-fitting `T` is always
     * heap-allocated whenever this vtable is active, so `src.heap` is
     * guaranteed non-null here.
     */
    static void move(CallableStorage& dst, CallableStorage& src) noexcept {
        if constexpr (SBOTraits<T>::fits) {
            T* srcPtr = static_cast<T*>(src.inlineSlot());
            new (dst.inlineSlot()) T(std::move(*srcPtr));
            srcPtr->~T();
        } else {
            *dst.heapSlot() = src.heap;
            src.heap = nullptr;
        }
    }

    /**
     * @brief Destroys the stored callable and releases owned resources.
     * @param storage Storage holding the live `T` instance to destroy.
     */
    static void destroy(CallableStorage& storage) noexcept {
        if constexpr (SBOTraits<T>::fits) {
            static_cast<T*>(storage.inlineSlot())->~T();
        } else {
            delete static_cast<T*>(storage.heap);
        }
    }

    /**
     * @brief Returns the shared vtable for copyable callables.
     * @return Pointer to a function-local `static constexpr` `VTable`,
     * shared by every `Function` holding a `T`.
     */
    static const VTable<R, Args...>* get() noexcept {
        static constexpr VTable<R, Args...> vtable{&invoke, &copy, &move, &destroy};
        return &vtable;
    }

    /**
     * @brief Returns the shared vtable for move-only callables.
     * @return Pointer to a function-local `static constexpr` `VTable`
     * with `copy` set to `nullptr`, shared by every `MoveOnlyFunction`
     * holding a `T`.
     */
    static const VTable<R, Args...>* getMoveOnly() noexcept {
        static constexpr VTable<R, Args...> vtable{&invoke, nullptr, &move, &destroy};
        return &vtable;
    }
};

} // namespace FunctionPro::Detail
