#pragma once

#include "api.hpp"
#include "types.hpp"

namespace i2c::strs {
    template <typename T>
    concept convertible_to_il2cpp =
        std::is_constructible_v<std::string_view, T> || std::is_constructible_v<std::u16string_view, T> || std::is_same_v<System::String*, T>;

    void convstr(char const* inp, char16_t* outp, int sz);
    std::size_t convstr(char16_t const* inp, char* outp, int isz, int osz);

    inline std::string to_string(System::String* str) {
        std::string val(str->length * sizeof(wchar_t) + 1, '\0');
        auto resSize = convstr(str->chars, val.data(), str->length, val.size());
        val.resize(resSize);
        return val;
    }
    inline std::u16string to_u16string(System::String* str) {
        return {str->chars, static_cast<std::size_t>(str->length)};
    }
    inline std::wstring to_wstring(System::String* str) {
        return {str->chars, str->chars + str->length};
    }
    inline std::u16string_view to_u16string_view(System::String* inst) {
        return {inst->chars, inst->chars + inst->length};
    }
    inline std::u16string_view to_u16string_view(System::String const* inst) {
        return {inst->chars, inst->chars + inst->length};
    }

    System::String* alloc_str(std::string_view str);
    System::String* alloc_str(std::u16string_view str);

    System::String* strappend(std::string_view const lhs, System::String const* rhs) noexcept;
    System::String* strappend(std::u16string_view const lhs, System::String const* rhs) noexcept;
    System::String* strappend(System::String const* lhs, std::string_view const rhs) noexcept;
    System::String* strappend(System::String const* lhs, std::u16string_view const rhs) noexcept;
    System::String* strappend(System::String const* lhs, System::String const* rhs) noexcept;

    bool strcomp(System::String const* lhs, std::string_view const rhs) noexcept;
    bool strcomp(System::String const* lhs, std::u16string_view const rhs) noexcept;
    bool strcomp(System::String const* lhs, System::String const* rhs) noexcept;

    bool strless(System::String const* lhs, std::string_view const rhs) noexcept;
    bool strless(System::String const* lhs, std::u16string_view const rhs) noexcept;
    bool strless(System::String const* lhs, System::String const* rhs) noexcept;

    bool strstart(System::String const* lhs, std::string_view const rhs) noexcept;
    bool strstart(System::String const* lhs, std::u16string_view const rhs) noexcept;
    bool strstart(System::String const* lhs, System::String const* rhs) noexcept;

    bool strend(System::String const* lhs, std::string_view const rhs) noexcept;
    bool strend(System::String const* lhs, std::u16string_view const rhs) noexcept;
    bool strend(System::String const* lhs, System::String const* rhs) noexcept;
}

// C# strings can only have 'int' max length.
template <int sz>
struct ConstString {
    // Manually allocated string, dtor destructs in place
    ConstString(char const (&st)[sz]) { i2c::strs::convstr(st, chars, length); }
    constexpr ConstString(char16_t const (&st)[sz]) noexcept {
        for (int i = 0; i < length; i++) {
            chars[i] = st[i];
        }
    }
    // Copies allowed? But should probably be avoided.
    ConstString(ConstString const&) noexcept = default;
    // Moves allowed
    ConstString(ConstString&&) noexcept = default;

    void init() noexcept { klass = i2c::functions::defaults->string_class; }

    constexpr operator System::String*() {
        if (!klass) {
            klass = i2c::functions::defaults->string_class;
        }
        return unsafe_cast();
    }

    constexpr operator System::String const*() const {
        if (!klass) {
            const_cast<ConstString<sz>*>(this)->klass = i2c::functions::defaults->string_class;
        }
        return unsafe_cast();
    }

    constexpr System::String* operator->() { return operator System::String*(); }

    operator std::string() const { return i2c::strs::to_string(unsafe_cast()); }
    operator std::u16string() const { return i2c::strs::to_u16string(unsafe_cast()); }
    operator std::wstring() const { return i2c::strs::to_wstring(unsafe_cast()); }
    constexpr operator std::u16string_view const() const { return {chars, static_cast<std::size_t>(sz)}; }
    constexpr operator std::u16string_view() { return {chars, static_cast<std::size_t>(sz)}; }

   private:
    void* klass = nullptr;
    void* monitor = nullptr;
    int length = sz - 1;
    char16_t chars[sz] = {};

    System::String* unsafe_cast() { return reinterpret_cast<System::String*>(&klass); }
    System::String const* unsafe_cast() const { return reinterpret_cast<System::String const*>(&klass); }
};

struct StringW {
    using ptr = System::String*;
    using const_ptr = System::String const*;

    using value = Il2CppChar;
    using const_value = Il2CppChar const;
    using pointer = Il2CppChar*;
    using const_pointer = Il2CppChar const*;
    using reference = Il2CppChar&;
    using const_reference = Il2CppChar const&;

    using iterator = pointer;
    using const_iterator = const_pointer;

    constexpr StringW() noexcept : inst(nullptr) {}
    constexpr StringW(std::nullptr_t npt) noexcept : inst(npt) {}
    constexpr StringW(void* ins) noexcept : inst(static_cast<ptr>(ins)) {}
    constexpr StringW(ptr ins) noexcept : inst(ins) {}
    template <int sz>
    constexpr StringW(ConstString<sz>& str) noexcept : inst(static_cast<ptr>(str)) {}
    // Dynamically allocated string
    template <i2c::strs::convertible_to_il2cpp T>
    StringW(T str) : inst(i2c::strs::alloc_str(str)) {}

    constexpr StringW(StringW const&) noexcept = default;
    constexpr StringW(StringW&&) noexcept = default;

    constexpr void* convert() const noexcept { return const_cast<void*>(static_cast<void*>(inst)); }

    constexpr operator ptr() const noexcept { return inst; }
    constexpr operator const_ptr() const noexcept { return inst; }

    constexpr ptr operator->() noexcept { return inst; }
    constexpr const_ptr operator->() const noexcept { return inst; }

    constexpr operator bool() const noexcept { return inst != nullptr; }
    constexpr bool operator==(std::nullptr_t rhs) const noexcept { return inst == rhs; }

    template <i2c::strs::convertible_to_il2cpp T>
    StringW& operator+=(T const& rhs) noexcept {
        inst = i2c::strs::strappend(inst, rhs);
        return *this;
    }
    StringW& operator+=(StringW const& rhs) noexcept { return this->operator+=(rhs.inst); }

    template <i2c::strs::convertible_to_il2cpp T>
    StringW operator+(T const& rhs) const noexcept {
        return i2c::strs::strappend(inst, rhs);
    }
    StringW operator+(StringW const& rhs) const noexcept { return this->operator+(rhs.inst); }

    template <i2c::strs::convertible_to_il2cpp T>
    bool operator<(T const& rhs) const noexcept {
        return i2c::strs::strless(inst, rhs);
    }
    bool operator<(StringW const& rhs) const noexcept { return this->operator<(rhs.inst); }

    template <i2c::strs::convertible_to_il2cpp T>
    bool operator==(T const& rhs) const noexcept {
        return i2c::strs::strcomp(inst, rhs);
    }
    bool operator==(StringW const& rhs) const noexcept { return this->operator==(rhs.inst); }

    template <i2c::strs::convertible_to_il2cpp T>
    bool starts_with(T const& rhs) const noexcept {
        return i2c::strs::strstart(inst, rhs);
    }
    bool starts_with(StringW const& rhs) const noexcept { return starts_with(rhs.inst); }

    template <i2c::strs::convertible_to_il2cpp T>
    bool ends_with(T const& rhs) const noexcept {
        return i2c::strs::strend(inst, rhs);
    }
    bool ends_with(StringW const& rhs) const noexcept { return ends_with(rhs.inst); }

    iterator begin() { return inst->chars; }
    const_iterator begin() const { return inst->chars; }
    iterator end() { return inst->chars + inst->length; }
    const_iterator end() const { return inst->chars + inst->length; }

    operator std::span<value>() { return {begin(), end()}; }
    operator std::span<const_value>() const { return {begin(), end()}; }

    reference operator[](size_t const& idx) { return inst->chars[idx]; }
    const_reference operator[](size_t const& idx) const { return inst->chars[idx]; }

    operator std::string() const { return i2c::strs::to_string(inst); }
    operator std::u16string() const { return i2c::strs::to_u16string(inst); }
    operator std::wstring() const { return i2c::strs::to_wstring(inst); }
    operator std::u16string_view const() const { return i2c::strs::to_u16string_view(inst); }
    operator std::u16string_view() { return i2c::strs::to_u16string_view(inst); }

   private:
    ptr inst;
};

template <i2c::strs::convertible_to_il2cpp T>
StringW operator+(T const lhs, StringW const& rhs) noexcept {
    return i2c::strs::strappend(lhs, static_cast<System::String const*>(rhs));
}

DEFINE_IL2CPP_DEFAULT_CLASS_REF(StringW, string);

static_assert(sizeof(StringW) == sizeof(void*));
static_assert(i2c::type_check::wrapper_ref_type<StringW>);

inline std::string format_as(StringW str) {
    if (!str) {
        return "StringW(null)";
    }
    return str;
}
