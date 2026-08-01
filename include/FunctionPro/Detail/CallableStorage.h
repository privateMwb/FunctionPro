/**
 * @file CallableStorage.h
 * @brief Raw inline/heap storage backing FunctionPro's type-erased callables.
 *
 * Contains the storage primitive shared by `Function`, `MoveOnlyFunction`,
 * and `VTableFactory` for holding a type-erased callable either inline
 * (Small Buffer Optimization) or on the heap.
 */

#pragma once

// clang-format off
#include <cstddef>                  // std::size_t, std::byte
// clang-format on

namespace FunctionPro::Detail {

/**
 * @brief Inline-or-heap storage for a single type-erased callable.
 * @details `buffer` and `heap` are mutually exclusive for any given
 * stored callable — a callable is either small enough to live inline, or
 * it lives on the heap, never both — so they are unioned together. This
 * reclaims the bytes that would otherwise be permanently spent on the
 * heap pointer, growing the usable inline (SBO) capacity by
 * `sizeof(void*)` at no cost to the overall footprint of
 * `CallableStorage`.
 *
 * Which member is active is not tracked at runtime: there is no
 * discriminant and no branch on it. It is known at compile time by
 * whichever `VTableFactory<T, ...>` instantiation is operating on the
 * storage, via `Detail::SBOTraits<T>::fits`. This keeps
 * invoke/copy/move/destroy free of any runtime "inline vs heap" check.
 */
struct CallableStorage {
    /// @brief Required alignment of the inline buffer.
    static constexpr std::size_t SBO_ALIGNMENT = 8;

    /// @brief Inline capacity. Equals the base 32-byte buffer plus the
    /// bytes reclaimed from unioning away the separate heap pointer field.
    static constexpr std::size_t SBO_SIZE = 32 + sizeof(void*);

    /// @brief The active storage: either the inline SBO buffer, or the
    /// pointer to a heap allocation. Exactly one is live at a time.
    union {
        alignas(SBO_ALIGNMENT) std::byte buffer[SBO_SIZE]; ///< Inline (SBO) storage.
        void* heap; ///< Pointer to the heap allocation, when not stored inline.
    };

    /// @brief Initializes storage to the empty state (`heap == nullptr`).
    constexpr CallableStorage() noexcept : heap(nullptr) {}

    /**
     * @brief Returns the address of the inline storage buffer.
     * @return Pointer to the inline buffer, for placement construction or
     * access of an inline-stored callable.
     */
    [[nodiscard]] constexpr void* inlineSlot() noexcept {
        return buffer;
    }

    /// @copydoc inlineSlot()
    [[nodiscard]] constexpr const void* inlineSlot() const noexcept {
        return buffer;
    }

    /**
     * @brief Returns the address of the heap pointer itself.
     * @return Pointer to the `heap` member, for storing, transferring, or
     * clearing the heap allocation's pointer.
     */
    [[nodiscard]] constexpr void** heapSlot() noexcept {
        return &heap;
    }
};

} // namespace FunctionPro::Detail
