#pragma once

#include "types.hpp"

/// @brief Represents a byref parameter that wraps a reference.
/// Use by_ref<T const> for C# readonly/in parameters. This wrapper does not extend the referent's lifetime.
/// This is REQUIRED for codegen invokes, as run_method can't tell the difference between a reference parameter and a byref on constexpr time.
template <typename T>
requires(!std::is_reference_v<T>)
struct by_ref {
    constexpr by_ref(T& val) noexcept : ref(&val) {}
    explicit constexpr by_ref(void* val) noexcept : ref(reinterpret_cast<T*>(val)) {}

    template <typename U>
    requires(std::is_same_v<T, U const>)
    constexpr by_ref(by_ref<U> other) noexcept : ref(other.ref) {}

    // runtime_invoke uses void* for both mutable and readonly argument storage.
    constexpr void* convert() const noexcept { return const_cast<void*>(static_cast<void const*>(ref)); }

    /// @brief Returns a readonly view of the same referent.
    constexpr by_ref<T const> as_const() const noexcept { return *this; }

    /// @brief Removes only the referent's top-level constness, like const_cast.
    /// The referent must be valid; writing through this view requires an originally mutable object.
    constexpr by_ref<std::remove_const_t<T>> as_mutable() const noexcept { return const_cast<std::remove_const_t<T>&>(*ref); }

    constexpr T& operator*() noexcept { return *ref; }
    constexpr T const& operator*() const noexcept { return *ref; }
    constexpr T* operator->() noexcept { return ref; }
    constexpr T const* operator->() const noexcept { return ref; }

    by_ref<T>& operator=(T& other) {
        ref = &other;
        return *this;
    }

    T* ref;
};

// We do not need a no_arg_class specialization for by_ref, since it will never get to that point.
template <typename T>
struct BS_HOOK_HIDDEN ::i2c::type_check::no_arg_type<by_ref<T>> {
    static inline Il2CppType const* get() { return &no_arg_class<std::remove_const_t<T>>::get()->this_arg; }
};
MARK_GEN_REF_T(by_ref);
