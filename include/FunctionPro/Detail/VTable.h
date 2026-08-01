/**
 * @file VTable.h
 * @brief Type-erased operation table used by FunctionPro's callable wrappers.
 *
 * Contains the dispatch table structure that `VTableFactory` populates
 * per concrete callable type, and that `Function`/`MoveOnlyFunction`
 * hold a pointer to for invoking, copying, moving, and destroying their
 * stored callable without knowing its concrete type.
 */

#pragma once

// clang-format off
#include "CallableStorage.h"        // CallableStorage — storage type every VTable operation receives a reference to
// clang-format on

namespace FunctionPro::Detail {

/**
 * @brief Type-erased dispatch table for callable objects.
 * @tparam R Return type of the erased callable's call signature.
 * @tparam Args Argument types of the erased callable's call signature.
 * @details Stores the operations required to invoke, copy, move, and
 * destroy a specific callable type. Each operation receives a
 * `CallableStorage` reference, allowing the implementation to
 * transparently handle both SBO and heap storage. A single `VTable`
 * instance is shared (as a `static constexpr` local in `VTableFactory`)
 * by every `Function`/`MoveOnlyFunction` holding the same concrete
 * callable type, so binding a callable costs one pointer store, not a
 * per-instance vtable allocation.
 *
 * `move` and `destroy` are `noexcept`: a stored callable's move
 * constructor and destructor are assumed not to throw. This mirrors an
 * assumption already implicit elsewhere in this library —
 * `Function`/`MoveOnlyFunction`'s move constructor and move-assignment
 * operator are themselves declared `noexcept` and call straight through
 * to these operations. Declaring them `noexcept` here too makes that
 * contract explicit, lets the compiler drop unwinding machinery from the
 * hot move/destroy paths, and is relied on by `swap()`.
 */
template <typename R, typename... Args> struct VTable {

    /// @brief Invokes the stored callable.
    R (*invoke)(CallableStorage&, Args&&...);

    /// @brief Copy-constructs the callable into the destination storage.
    /// May throw (propagates the callable's copy constructor exceptions).
    /// `nullptr` for move-only callables.
    void (*copy)(CallableStorage&, const CallableStorage&);

    /// @brief Move-constructs the callable into the destination storage
    /// and ends the source callable's lifetime.
    void (*move)(CallableStorage&, CallableStorage&) noexcept;

    /// @brief Destroys the stored callable and releases any owned resources.
    void (*destroy)(CallableStorage&) noexcept;
};

} // namespace FunctionPro::Detail
