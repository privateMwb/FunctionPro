#pragma once

#include <cstddef>

// Storage
// raw SBO buffer and heap pointer used by callable wrappers
struct Storage {
    static constexpr std::size_t SBO_SIZE = 32;
    static constexpr std::size_t SBO_ALIGNMENT = 8;

    alignas(SBO_ALIGNMENT) std::byte buffer[SBO_SIZE];
    void* heap = nullptr;

    [[nodiscard]] void* get() noexcept {
        return heap ? heap : static_cast<void*>(buffer);
    }

    [[nodiscard]] const void* get() const noexcept {
        return heap ? heap : static_cast<const void*>(buffer);
    }
};