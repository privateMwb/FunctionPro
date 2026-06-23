#pragma once

#include <utility>

// VTableFactory
// builds VTable instances for a concrete callable type template
template<typename T,
	typename R,
	typename... Args>
struct VTableFactory {
	static R invoke(void* storage, Args&&... args) {
		return (*static_cast<T*>(storage))(std::forward<Args>(args)...);
	}

	static void copy(void* dst, const void* src) {
		if constexpr (SBOTraits<T>::fits) {
			new (dst) T(*static_cast<const T*>(src));
		}
		else {
			*static_cast<void**>(dst) = new T(*static_cast<const T*>(src));
		}
	}

	static void move(void* dst, void* src, bool fromHeap) {
		if constexpr (SBOTraits<T>::fits) {
			new (dst) T(std::move(*static_cast<T*>(src)));
		}
		else {
			if (fromHeap) {
				*static_cast<void**>(dst) = *static_cast<void**>(src);
				*static_cast<void**>(src) = nullptr;
			}
			else {
				*static_cast<void**>(dst) = new T(std::move(*static_cast<T*>(src)));
			}
		}
	}

	static void destroy(void* storage) {
		if constexpr (SBOTraits<T>::fits) {
			static_cast<T*>(storage)->~T();
		}
		else {
			delete static_cast<T*>(storage);
		}
	}

	static const VTable<R, Args...>* get() noexcept {
		static constexpr VTable<R, Args...> vtable{
			&invoke, &copy, &move, &destroy
		};
		return &vtable;
	}

	static const VTable<R, Args...>* getMoveOnly() noexcept {
		static constexpr VTable<R, Args...> vtable{
			&invoke, nullptr, &move, &destroy
		};
		return &vtable;
	}
};