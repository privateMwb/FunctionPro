/**
 * @file SBOTraits.h
 * @brief Small Buffer Optimization fit-test trait used by FunctionPro.
 *
 * Contains the trait used internally by `Function`, `MoveOnlyFunction`,
 * and `VTableFactory` to decide, at compile time, whether a given
 * callable type can be stored inline in `CallableStorage` or must be
 * heap-allocated.
 */

#pragma once

// clang-format off
#include "CallableStorage.h"        // CallableStorage — SBO/heap storage the trait compares sizeof(T)/alignof(T) against
// clang-format on

namespace FunctionPro::Detail {

/**
 * @brief Determines whether a callable type fits in the inline SBO buffer.
 * @tparam T Callable type being tested.
 * @details Compared against `CallableStorage::SBO_SIZE` and
 * `CallableStorage::SBO_ALIGNMENT`. Every operation in `VTableFactory` is
 * instantiated per concrete `T`, so `fits` is resolved entirely at
 * compile time via `if constexpr` — there is never a runtime check of
 * where a given callable actually lives.
 */
template <typename T> struct SBOTraits {
    static constexpr bool fits =
        sizeof(T) <= CallableStorage::SBO_SIZE && alignof(T) <= CallableStorage::SBO_ALIGNMENT;
};

} // namespace FunctionPro::Detail
