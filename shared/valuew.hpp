#pragma once

#include "types.hpp"

// A type that (unsafely) represents a C# value type
template <size_t S, i2c::str_lit Namespace, i2c::str_lit Name>
struct ValueW {
    constexpr ValueW() = default;
    constexpr ValueW(ValueW const&) = default;
    constexpr ValueW(ValueW&&) = default;

    constexpr explicit ValueW(std::array<std::byte, S> data) noexcept : instance(std::move(data)) {}
    constexpr ValueW(void* data) noexcept { std::copy_n(reinterpret_cast<std::byte*>(data), S, instance.data()); }

    void* convert() const noexcept { return const_cast<void*>(static_cast<void const*>(instance.data())); }

    constexpr ValueW& operator=(ValueW&&) = default;
    constexpr ValueW& operator=(ValueW const&) = default;

    std::array<std::byte, S> instance;
};

template <size_t S, i2c::str_lit Namespace, i2c::str_lit Name>
struct i2c::type_check::no_arg_class<ValueW<S, Namespace, Name>> {
    static inline Il2CppClass* get() { return class_of<const_type<Namespace, Name>>(); }
};
template <size_t S, i2c::str_lit Namespace, i2c::str_lit Name>
struct i2c::type_markers::value_type_trait<ValueW<S, Namespace, Name>> {
    static constexpr bool value = true;
};
