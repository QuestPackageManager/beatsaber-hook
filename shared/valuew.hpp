#pragma once

#include "types.hpp"

template <size_t S, i2c::str_lit Namespace, i2c::str_lit Name>
struct ValueW {
    constexpr ValueW() = default;
    constexpr ValueW(ValueW const&) = default;
    constexpr ValueW(ValueW&&) = default;

    constexpr explicit ValueW(std::array<std::byte, S> data) noexcept : instance(std::move(data)) {}
    constexpr ValueW(void* data) noexcept { std::copy_n(reinterpret_cast<char*>(data), S, instance.data()); }

    void* convert() const noexcept { return const_cast<void*>(static_cast<void const*>(instance.data())); }

    constexpr ValueW& operator=(ValueW&& o) = default;
    constexpr ValueW& operator=(ValueW const& o) = default;

    static constexpr size_t Size = S;
    static constexpr i2c::str_lit Ns = Namespace;
    static constexpr i2c::str_lit Nm = Name;
    std::array<std::byte, S> instance;
};

// Types inheriting from ValueW will also inherit its type markers.
template <typename T>
requires(std::derived_from<T, ValueW<T::Size, T::Ns, T::Nm>>)
struct i2c::type_check::no_arg_class<T> {
    static inline Il2CppClass* get() { return class_of<const_type<T::Ns, T::Nm>>(); }
};
template <typename T>
requires(std::derived_from<T, ValueW<T::Size, T::Ns, T::Nm>>)
struct i2c::type_markers::value_type_trait<T> {
    static constexpr bool value = true;
};
