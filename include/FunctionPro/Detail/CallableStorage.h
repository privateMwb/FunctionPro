#pragma once

#include <cstddef>

namespace FunctionPro::Detail {

// Callable storage used by FunctionPro wrappers.
//
// Three distinct accessors, each with a single responsibility:
//
//   data()       -> Returns the address of the active callable,
//                   whether stored inline or on the heap.
//
//   inlineSlot() -> Returns the address of the SBO buffer.
//                   Used only for placement construction.
//
//   heapSlot()   -> Returns the address of the heap pointer itself.
//                   Used only when storing, moving, or clearing the pointer.

struct CallableStorage {
    static constexpr std::size_t SBO_SIZE      = 32;
    static constexpr std::size_t SBO_ALIGNMENT = 8;

    // Inline storage used for SBO.
    alignas(SBO_ALIGNMENT) std::byte buffer[SBO_SIZE];

    // Heap storage used when the callable does not fit in the SBO buffer.
    void* heap = nullptr;

    // Returns the active callable.
    [[nodiscard]] constexpr void* data() noexcept {
        return heap ? heap : static_cast<void*>(buffer);
    }

    [[nodiscard]] constexpr const void* data() const noexcept {
        return heap ? heap : static_cast<const void*>(buffer);
    }

    // Returns the inline storage buffer.
    [[nodiscard]] constexpr void* inlineSlot() noexcept {
        return buffer;
    }

    // Returns the address of the heap pointer.
    [[nodiscard]] constexpr void** heapSlot() noexcept {
        return &heap;
    }
};

} // namespace FunctionPro::Detail