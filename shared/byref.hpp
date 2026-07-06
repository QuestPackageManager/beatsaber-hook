#pragma once

#include "types.hpp"

/// @brief Represents a byref parameter that wraps a reference.
/// This is REQUIRED for codegen invokes, as run_method can't tell the difference between a reference parameter and a byref on constexpr time.
template <typename T>
requires(!std::is_reference_v<T>)
struct by_ref {
    constexpr by_ref(T& val) noexcept : ref(&val) {}
    explicit constexpr by_ref(void* val) noexcept : ref(reinterpret_cast<T*>(val)) {}

    constexpr void* convert() const noexcept { return reinterpret_cast<void*>(ref); }

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
    static inline Il2CppType const* get() { return &no_arg_class<T>::get()->this_arg; }
};
MARK_GEN_REF_T(by_ref);
