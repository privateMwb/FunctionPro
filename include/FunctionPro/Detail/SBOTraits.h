/**
 * @file            SBOTraits.h
 *
 * @date            2026-23-7
 *
 * @version         1.0.0
 *
 * @copyright       Copyright (c) 2026 privateMwb
 *                  All rights reserved.
 *                  https://github.com/privateMwb/FunctionPro
 *
 * @attention       This source is released under the MIT license
 *                  SPDX-License-Identifier: MIT
 *                  <http://opensource.org/licenses/MIT>
 */

#pragma once

// clang-format off
#include "CallableStorage.h"        // CallableStorage — SBO/heap storage the trait compares sizeof(T)/alignof(T) against
// clang-format on

namespace FunctionPro::Detail {

/**
 * @brief Determines whether a callable type fits in the inline SBO buffer.
 * @tparam T Callable type being tested.
 * @details Used internally by `Function`, `MoveOnlyFunction`, and
 * `VTableFactory` to decide, at compile time, whether a given callable
 * type can be stored inline in `CallableStorage` or must be
 * heap-allocated. Compared against `CallableStorage::SBO_SIZE` and
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
