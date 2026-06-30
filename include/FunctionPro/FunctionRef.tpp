#include <stdexcept>

namespace FunctionPro {

// Constructor
template<typename R, typename... Args>
template<typename T, typename>
FunctionRef<R(Args...)>::FunctionRef(T&& callable) noexcept
    : ptr_(reinterpret_cast<void*>(std::addressof(callable)))
    , invoke_([](void* ptr, Args&&... args) -> R {
        return (*reinterpret_cast<std::remove_reference_t<T>*>(ptr))(
            std::forward<Args>(args)...);
    }) {}

// Invocation
template<typename R, typename... Args>
R FunctionRef<R(Args...)>::operator()(Args... args) const {
    if (!invoke_) throw std::bad_function_call{};
    return invoke_(ptr_, std::forward<Args>(args)...);
}

// State
template<typename R, typename... Args>
FunctionRef<R(Args...)>::operator bool() const noexcept {
    return invoke_ != nullptr;
}

// Equality
template<typename R, typename... Args>
bool FunctionRef<R(Args...)>::operator==(std::nullptr_t) const noexcept {
    return invoke_ == nullptr;
}

template<typename R, typename... Args>
bool FunctionRef<R(Args...)>::operator!=(std::nullptr_t) const noexcept {
    return invoke_ != nullptr;
}

} // namespace FunctionPro
