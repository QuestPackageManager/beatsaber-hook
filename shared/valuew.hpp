#pragma once

#include "types.hpp"

template <size_t S>
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
    std::array<std::byte, S> instance;
};

template <size_t S>
struct i2c::type_check::no_arg_class<ValueW<S>> {
    static inline Il2CppClass* get() {
        static Il2CppClass* klass = get_class_from_name("System", "ValueType");
        return klass;
    }
};
// Types inheriting from ValueW will also automatically be marked as value types.
template <typename T>
requires(std::derived_from<T, ValueW<T::Size>>)
struct i2c::type_markers::value_type_trait<T> {
    static constexpr bool value = true;
};
