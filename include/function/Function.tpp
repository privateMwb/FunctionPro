#include <stdexcept>
#include <functional>

namespace FunctionPro {

// Constructors & Destructor
template<typename R, typename... Args>
Function<R(Args...)>::Function(std::nullptr_t) noexcept
: vtable_(nullptr) {}

template<typename R, typename... Args>
template<typename T, typename>
Function<R(Args...)>::Function(T&& callable) {
	using DecayT = std::decay_t<T>;

	if constexpr (SBOTraits<DecayT>::fits) {
		new (storage_.buffer) DecayT(std::forward<T>(callable));
	} else {
		storage_.heap = new DecayT(std::forward<T>(callable));
	}

	vtable_ = VTableFactory<DecayT, R, Args...>::get();
}

template<typename R, typename... Args>
Function<R(Args...)>::~Function() {
	reset();
}

template<typename R, typename... Args>
Function<R(Args...)>::Function(const Function<R(Args...)>& other) {
    if (other.vtable_) {
        vtable_ = other.vtable_;
        vtable_->copy(storage_.get(), other.storage_.get());
        storage_.heap = other.storage_.heap;
    }
}

template<typename R, typename... Args>
Function<R(Args...)>& Function<R(Args...)>::operator=(const Function<R(Args...)>& other) {
    if (this != &other) {
        reset();
        if (other.vtable_) {
            vtable_ = other.vtable_;
            vtable_->copy(storage_.get(), other.storage_.get());
            storage_.heap = other.storage_.heap;
        }
    }
    return *this;
}

template<typename R, typename... Args>
Function<R(Args...)>::Function(Function<R(Args...)>&& other) noexcept {
	if (other.vtable_) {
		vtable_ = other.vtable_;
		bool fromHeap = other.storage_.heap != nullptr;
		vtable_->move(storage_.get(), other.storage_.get(), fromHeap);
		if (fromHeap) storage_.heap = other.storage_.heap;

		other.vtable_       = nullptr;
		other.storage_.heap = nullptr;
	}
}

template<typename R, typename... Args>
Function<R(Args...)>& Function<R(Args...)>::operator=(Function<R(Args...)>&& other) noexcept {
	if (this != &other) {
		reset();

		if (other.vtable_) {
			vtable_ = other.vtable_;
			bool fromHeap = other.storage_.heap != nullptr;
			vtable_->move(storage_.get(), other.storage_.get(), fromHeap);
			if (fromHeap) storage_.heap = other.storage_.heap;

			other.vtable_       = nullptr;
			other.storage_.heap = nullptr;
		}
	}
	return *this;
}

// Invocation
template<typename R, typename... Args>
R Function<R(Args...)>::operator()(Args... args) const {
	if (!vtable_) throw std::bad_function_call{};
	return vtable_->invoke(
	           const_cast<void*>(storage_.get()),
	           std::forward<Args>(args)...);
}

// State
template<typename R, typename... Args>
Function<R(Args...)>::operator bool() const noexcept {
	return vtable_ != nullptr;
}

template<typename R, typename... Args>
void Function<R(Args...)>::reset() noexcept {
	if (vtable_) {
		vtable_->destroy(storage_.get());
		vtable_       = nullptr;
		storage_.heap = nullptr;
	}
}

// Equality
template<typename R, typename... Args>
bool Function<R(Args...)>::operator==(std::nullptr_t) const noexcept {
	return vtable_ == nullptr;
}

template<typename R, typename... Args>
bool Function<R(Args...)>::operator!=(std::nullptr_t) const noexcept {
	return vtable_ != nullptr;
}

} // namespace FunctionPro
