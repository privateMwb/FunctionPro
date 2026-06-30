#pragma once

#include "CallableStorage.h"

namespace FunctionPro::Detail {

// Determines whether a callable can be stored using
// Small Buffer Optimization (SBO).
template<typename T>
struct SBOTraits {
    static constexpr bool fits =
        sizeof(T)  <= CallableStorage::SBO_SIZE &&
        alignof(T) <= CallableStorage::SBO_ALIGNMENT;
};

} // namespace FunctionPro::Detail