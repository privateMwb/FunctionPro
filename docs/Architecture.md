# FunctionPro Architecture

## Overview

FunctionPro provides three callable wrapper types built on a shared
type-erasure foundation:

- `Function<R(Args...)>`         — copyable, owning callable wrapper
- `MoveOnlyFunction<R(Args...)>` — move-only, owning callable wrapper
- `FunctionRef<R(Args...)>`      — non-owning, non-allocating callable view

All three share the same `detail/` layer: `CallableStorage`, `VTable`,
and `VTableFactory`. Only `FunctionRef` diverges at the storage level,
since it never owns the callable it refers to.

---

## Directory Structure

```
FunctionPro/
├── include/
│   └── FunctionPro/
│       ├── Function.h
│       ├── Function.tpp
│       ├── MoveOnlyFunction.h
│       ├── MoveOnlyFunction.tpp
│       ├── FunctionRef.h
│       ├── FunctionRef.tpp
│       └── detail/
│           ├── CallableStorage.h
│           ├── SBOTraits.h
│           ├── VTable.h
│           └── VTableFactory.h
├── tests/
├── benchmarks/
├── examples/
└── docs/
```

---

## Detail Layer

### CallableStorage

`CallableStorage` is the lowest-level component. It holds either an
inline SBO buffer or a heap pointer, and exposes three intent-revealing
accessors:

```
+--------------------------+
|  buffer[SBO_SIZE]        |  <- inline storage (32 bytes, aligned to 8)
|  void* heap              |  <- heap pointer (null when using SBO)
+--------------------------+
```

The three accessors each have exactly one job:

| Accessor       | Job                                                          | Used by                        |
|----------------|--------------------------------------------------------------|--------------------------------|
| `data()`       | Returns the address of the active callable, inline or heap   | `invoke`, `destroy`            |
| `inlineSlot()` | Returns `&buffer` unconditionally                            | Placement construction (SBO)   |
| `heapSlot()`   | Returns `&heap` unconditionally                              | Heap pointer store/transfer    |

The separation between `data()` and `heapSlot()` is the critical design
decision in this layer. Prior to the current design, a single `get()`
accessor served both jobs, which caused a class of bugs described in the
[Correctness](#correctness) section below.

---

### SBOTraits

`SBOTraits<T>` is a compile-time predicate that determines whether a
concrete callable type `T` fits within the SBO buffer:

```cpp
static constexpr bool fits =
    sizeof(T)  <= CallableStorage::SBO_SIZE &&
    alignof(T) <= CallableStorage::SBO_ALIGNMENT;
```

This is evaluated once per type `T` at instantiation time. Every
`if constexpr (SBOTraits<T>::fits)` branch in `VTableFactory` is a
compile-time decision — there is no runtime branching per invocation
to determine storage location.

---

### VTable

`VTable<R, Args...>` is a plain struct of four function pointers:

```cpp
struct VTable {
    R    (*invoke) (CallableStorage&, Args&&...);
    void (*copy)   (CallableStorage&, const CallableStorage&);
    void (*move)   (CallableStorage&, CallableStorage&);
    void (*destroy)(CallableStorage&);
};
```

Each function pointer receives a `CallableStorage&` rather than a raw
`void*`. This gives each operation full context to correctly resolve
the callable address (`data()`) or the heap slot (`heapSlot()`) as
needed, with no address ambiguity.

`MoveOnlyFunction` sets the `copy` slot to `nullptr`. No copy operation
is ever dispatched through it since copy construction and copy assignment
are deleted at the type level.

---

### VTableFactory

`VTableFactory<T, R, Args...>` generates the four operation functions
for a concrete callable type `T` and returns a pointer to a
`static constexpr VTable` instance:

```cpp
static const VTable<R, Args...>* get()         noexcept; // copy + move
static const VTable<R, Args...>* getMoveOnly() noexcept; // move only
```

Because the vtable is `static constexpr`, exactly one instance exists
per unique `(T, R, Args...)` combination across the entire program.
Every `Function` or `MoveOnlyFunction` wrapping the same callable type
shares the same vtable pointer — there is no per-instance vtable
allocation.

The four generated operations:

**invoke** — casts `storage.data()` to `T*` and calls `operator()`.

**copy** — branches on `SBOTraits<T>::fits` at compile time:
- SBO: placement-constructs a new `T` into `dst.inlineSlot()` by
  copy-constructing from `*static_cast<const T*>(src.data())`.
- Heap: allocates a new `T` via `new T(...)` and stores the pointer
  into `*dst.heapSlot()`.

**move** — branches on `SBOTraits<T>::fits` at compile time:
- SBO: placement-constructs a new `T` into `dst.inlineSlot()` by
  move-constructing from `*static_cast<T*>(src.data())`.
- Heap: transfers the pointer value — `*dst.heapSlot() = *src.heapSlot()`
  — then nulls the source slot via `*src.heapSlot() = nullptr`. The
  pointed-to object's memory is never touched during a heap move.

**destroy** — branches on `SBOTraits<T>::fits` at compile time:
- SBO: calls `~T()` directly on `storage.data()`.
- Heap: calls `delete static_cast<T*>(storage.data())`.

---

## Owning Wrappers: Function and MoveOnlyFunction

Both owning wrappers share the same two-member layout:

```
+-----------------------------------+
|  const VTable<R,Args...>* vtable_ |  8 bytes (pointer)
+-----------------------------------+
|  CallableStorage storage_         |  40 bytes (32 buffer + 8 heap ptr)
+-----------------------------------+
```

Total object size: 48 bytes.

`vtable_ == nullptr` is the canonical empty state. All state checks
(`operator bool`, `operator==`) test only `vtable_`.

### Construction

On construction from a callable `T`:

1. `SBOTraits<DecayT>::fits` is evaluated at compile time.
2. If true: `new (storage_.inlineSlot()) DecayT(std::forward<T>(callable))`.
3. If false: `*storage_.heapSlot() = new DecayT(std::forward<T>(callable))`.
4. `vtable_` is set to `VTableFactory<DecayT, R, Args...>::get()`.

### Copy (Function only)

Copy construction and copy assignment delegate entirely to
`vtable_->copy(storage_, other.storage_)`. No manual heap bookkeeping
is performed in `Function.tpp` — ownership transfer is fully encapsulated
in `VTableFactory::copy`.

`Function`'s template constructor requires `std::is_copy_constructible_v<T>`
as a constraint, ensuring that the `copy` thunk is always well-formed for
any `T` accepted by `Function`.

### Move

Move construction and move assignment delegate entirely to
`vtable_->move(storage_, other.storage_)`, then null `other.vtable_`.
The `move` thunk handles nulling `other.storage_.heap` for heap-stored
callables. No manual heap bookkeeping is performed in the `.tpp` files.

### Destruction and Reset

`reset()` calls `vtable_->destroy(storage_)`, then nulls both `vtable_`
and `storage_.heap`. The destroy thunk handles the actual object
destruction and deallocation.

### Invocation

`Function::operator()` is `const`-qualified to match `std::function`
semantics — a `const Function` can still invoke a mutable callable.
The `const_cast<CallableStorage&>(storage_)` inside `operator()` is
intentional and correct: constness applies to the wrapper, not to the
stored callable.

`MoveOnlyFunction::operator()` is non-`const`, matching
`std::move_only_function` semantics where invocation is a mutable
operation.

---

## Non-Owning Wrapper: FunctionRef

`FunctionRef` does not use `CallableStorage` or `VTableFactory`. It
stores two members:

```
+-----------------------------------+
|  PtrStorage ptr_                  |  8 bytes (union: obj ptr or fn ptr)
+-----------------------------------+
|  R (*invoke_)(PtrStorage, Args&&) |  8 bytes (function pointer)
+-----------------------------------+
```

Total object size: 16 bytes. No allocation, no ownership, no vtable.

### PtrStorage Union

```cpp
union PtrStorage {
    void*  obj;
    void (*fn)();
};
```

`void*` cannot portably hold a function pointer — only object pointers.
The union allows `FunctionRef` to store either an object pointer or a
function pointer in a standards-compliant way.

### Construction

The constructor takes `T&` (not `T&&`) to prevent binding to temporaries,
which would immediately produce a dangling pointer. Only named lvalues
with a lifetime the caller can reason about are accepted.

Construction branches at compile time:

- **Function pointer** (`std::is_pointer_v<DecayT> && std::is_function_v<...>`):
  stores via `ptr_.fn = reinterpret_cast<void(*)()>(callable)`.
- **Raw function type** (`std::is_function_v<std::remove_reference_t<T>>`):
  decays to a pointer first, then stores via `ptr_.fn`.
- **Callable object**: stores `&callable` via `ptr_.obj`.

The `invoke_` thunk is a stateless lambda that captures nothing and
converts to a plain function pointer. Each unique `T` produces a unique
thunk at compile time.

### Lifetime Contract

`FunctionRef` does not extend the lifetime of the callable it refers to.
The caller is responsible for ensuring the callable outlives the
`FunctionRef`. This is enforced at the API level by the `T&` constructor
— rvalues are rejected at compile time.

---

## Correctness

### The heapSlot / data Split

The most significant correctness issue addressed during development was
the conflation of two distinct addresses in the original `get()` accessor:

1. **The address of the live callable** — needed by `invoke` and `destroy`.
2. **The address of the heap pointer slot** — needed by `copy` and `move`
   when transferring heap ownership.

The original single `get()` returned `heap ? heap : buffer`. When called
on a freshly-constructed empty destination (where `heap == nullptr`),
it resolved to `&buffer` instead of `&heap`. The non-SBO branch of `copy`
then wrote the new heap pointer into the first bytes of `buffer`, which
was never read again. The actual `heap` member of the destination remained
null, and the caller patched it up manually with `storage_.heap = other.storage_.heap`,
which aliased the source pointer instead of storing the newly allocated one.
The result was a double-free on destruction.

The fix was to give the two jobs two different accessors (`data()` vs
`heapSlot()`), and to pass `CallableStorage&` through the vtable instead
of a raw `void*`, so each operation has access to both addresses without
ambiguity.

### FunctionRef Rvalue Binding

The original `FunctionRef` constructor accepted `T&&` (a forwarding
reference), which silently accepted temporaries:

```cpp
FunctionRef<int()> ref = []{ return 42; }; // dangling pointer
```

The lambda is destroyed at the end of the statement. Every subsequent
call through `ref` is undefined behavior. The fix is `T&` — only lvalues
are accepted, which are named objects whose lifetime the caller controls.

---

## SBO Sizing

The current SBO buffer is 32 bytes with 8-byte alignment, defined in
`CallableStorage`:

```cpp
static constexpr std::size_t SBO_SIZE      = 32;
static constexpr std::size_t SBO_ALIGNMENT = 8;
```

This accommodates most small lambdas with a few captures and typical
functor types. Callables exceeding either limit are heap-allocated
transparently. The constants are intentionally centralized in
`CallableStorage` so that a single change propagates to all
`SBOTraits<T>::fits` evaluations across the library.

---

## Compiler Requirements

- C++20 or later (concepts, `requires`, `if constexpr`)
- Tested on GCC, Clang, and MSVC

