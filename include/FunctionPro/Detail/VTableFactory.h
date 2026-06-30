#pragma once

#include "CallableStorage.h"
#include "VTable.h"
#include "SBOTraits.h"

#include <utility>

namespace FunctionPro::Detail {

// Creates the type-erased operation tables for a concrete callable type.
//
// Generates the functions required to invoke, copy, move, and destroy
// a stored callable while transparently handling both Small Buffer
// Optimization (SBO) and heap storage.

template<typename T,
         typename R,
         typename... Args>
struct VTableFactory {

    // Invokes the stored callable.
    static R invoke(CallableStorage& storage, Args&&... args) {
        return (*static_cast<T*>(storage.data()))(std::forward<Args>(args)...);
    }

    // Copy-constructs the callable into the destination storage.
    static void copy(CallableStorage& dst, const CallableStorage& src) {
        if constexpr (SBOTraits<T>::fits) {
            new (dst.inlineSlot()) T(*static_cast<const T*>(src.data()));
        } else {
            *dst.heapSlot() = new T(*static_cast<const T*>(src.data()));
        }
    }

    // Move-constructs the callable into the destination storage.
    static void move(CallableStorage& dst, CallableStorage& src) {
        if constexpr (SBOTraits<T>::fits) {
            new (dst.inlineSlot()) T(std::move(*static_cast<T*>(src.data())));
        } else {
            if (src.heap) {
                // Transfer ownership of the heap allocation.
                *dst.heapSlot() = *src.heapSlot();
                *src.heapSlot() = nullptr;
            } else {
                *dst.heapSlot() = new T(std::move(*static_cast<T*>(src.data())));
            }
        }
    }

    // Destroys the stored callable and releases owned resources.
    static void destroy(CallableStorage& storage) {
        if constexpr (SBOTraits<T>::fits) {
            static_cast<T*>(storage.data())->~T();
        } else {
            delete static_cast<T*>(storage.data());
        }
    }

    // Returns the shared vtable for copyable callables.
    static const VTable<R, Args...>* get() noexcept {
        static constexpr VTable<R, Args...> vtable{
            &invoke, &copy, &move, &destroy
        };
        return &vtable;
    }

    // Returns the shared vtable for move-only callables.
    static const VTable<R, Args...>* getMoveOnly() noexcept {
        static constexpr VTable<R, Args...> vtable{
            &invoke, nullptr, &move, &destroy
        };
        return &vtable;
    }
};

} // namespace FunctionPro::Detail