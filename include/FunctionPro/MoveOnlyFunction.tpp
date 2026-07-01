// ============================================================
// MoveOnlyFunction.tpp
// Template implementation for FunctionPro::MoveOnlyFunction.
// ============================================================
//
//  Sections:
//   1. Constructors & Destructor
//   2. Move Semantics
//   3. Invocation
//   4. State
//   5. Equality
//
// ============================================================

#include <stdexcept>
#include <functional>

namespace FunctionPro {

	// ============================================================
	//  Section 1 — Constructors & Destructor
	// ============================================================

	template<typename R, typename... Args>
	MoveOnlyFunction<R(Args...)>::MoveOnlyFunction(std::nullptr_t) noexcept
		: vtable_(nullptr) {
	}

	template<typename R, typename... Args>
	template<typename T>
		requires (!std::same_as<std::decay_t<T>, MoveOnlyFunction<R(Args...)>>)
	&& std::is_invocable_r_v<R, std::decay_t<T>, Args...>
		MoveOnlyFunction<R(Args...)>::MoveOnlyFunction(T&& callable) {
		using DecayT = std::decay_t<T>;

		// Store small callables inline; allocate larger ones on the heap.
		if constexpr (Detail::SBOTraits<DecayT>::fits) {
			new (storage_.inlineSlot()) DecayT(std::forward<T>(callable));
		}
		else {
			*storage_.heapSlot() = new DecayT(std::forward<T>(callable));
		}

		// Bind the callable's move-only type-erased operations.
		vtable_ = Detail::VTableFactory<DecayT, R, Args...>::getMoveOnly();
	}

	template<typename R, typename... Args>
	MoveOnlyFunction<R(Args...)>::~MoveOnlyFunction() {
		reset();
	}


	// ============================================================
	//  Section 2 — Move Semantics
	// ============================================================

	template<typename R, typename... Args>
	MoveOnlyFunction<R(Args...)>::MoveOnlyFunction(MoveOnlyFunction&& other) noexcept {

		// Transfer ownership of the stored callable.
		if (other.vtable_) {
			vtable_ = other.vtable_;
			vtable_->move(storage_, other.storage_);

			other.vtable_ = nullptr;
		}
	}

	template<typename R, typename... Args>
	MoveOnlyFunction<R(Args...)>&
		MoveOnlyFunction<R(Args...)>::operator=(MoveOnlyFunction&& other) noexcept {
		if (this != &other) {
			reset();

			// Transfer ownership from the source object.
			if (other.vtable_) {
				vtable_ = other.vtable_;
				vtable_->move(storage_, other.storage_);

				other.vtable_ = nullptr;
			}
		}

		return *this;
	}


	// ============================================================
	//  Section 3 — Invocation
	// ============================================================

	template<typename R, typename... Args>
	R MoveOnlyFunction<R(Args...)>::operator()(Args... args) {

		// Match std::function behavior for empty invocation.
		if (!vtable_)
			throw std::bad_function_call{};

		return vtable_->invoke(storage_, std::forward<Args>(args)...);
	}


	// ============================================================
	//  Section 4 — State
	// ============================================================

	template<typename R, typename... Args>
	MoveOnlyFunction<R(Args...)>::operator bool() const noexcept {
		return vtable_ != nullptr;
	}

	template<typename R, typename... Args>
	void MoveOnlyFunction<R(Args...)>::reset() noexcept {

		// Destroy the stored callable and restore the empty state.
		if (vtable_) {
			vtable_->destroy(storage_);
			vtable_ = nullptr;
			storage_.heap = nullptr;
		}
	}

	template<typename R, typename... Args>
	void MoveOnlyFunction<R(Args...)>::swap(MoveOnlyFunction& other) noexcept {

		// Exchange the stored callable state.
		std::swap(vtable_, other.vtable_);
		std::swap(storage_, other.storage_);
	}


	// ============================================================
	//  Section 5 — Equality
	// ============================================================

	template<typename R, typename... Args>
	bool MoveOnlyFunction<R(Args...)>::operator==(std::nullptr_t) const noexcept {
		return vtable_ == nullptr;
	}

	template<typename R, typename... Args>
	bool MoveOnlyFunction<R(Args...)>::operator!=(std::nullptr_t) const noexcept {
		return vtable_ != nullptr;
	}

} // namespace FunctionPro
