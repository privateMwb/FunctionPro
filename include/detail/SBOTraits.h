#pragma once

// SBOTraits
// detects whether a type fits within the SBO buffer
template<typename T>
struct SBOTraits {
    static constexpr bool fits =
        sizeof(T)  <= Storage::SBO_SIZE &&
        alignof(T) <= Storage::SBO_ALIGNMENT;
};