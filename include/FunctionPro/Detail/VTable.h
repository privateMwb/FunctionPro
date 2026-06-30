#pragma once

#include "CallableStorage.h"

namespace FunctionPro::Detail {

// Type-erased dispatch table for callable objects.
//
// Stores the operations required to invoke, copy, move, and destroy
// a specific callable type.
// Each operation receives a CallableStorage reference, allowing the
// implementation to transparently handle both SBO and heap storage.

template<typename R, typename... Args>
struct VTable {

    // Invokes the stored callable.
    R    (*invoke)(CallableStorage&, Args&&...);

    // Copy-constructs the callable into the destination storage.
    void (*copy)(CallableStorage&, const CallableStorage&);

    // Move-constructs the callable into the destination storage.
    void (*move)(CallableStorage&, CallableStorage&);

    // Destroys the stored callable and releases any owned resources.
    void (*destroy)(CallableStorage&);
};

} // namespace FunctionPro::Detail