#include "stringw.hpp"

#include "members.hpp"

template <typename Facet>
struct deletable_facet : Facet {
    template <typename... TArgs>
    deletable_facet(TArgs&&... args) : Facet(std::forward<TArgs>(args)...) {}
    ~deletable_facet() {}
};

// Note that char is actually required here over char8_t-- this is due to NDK not having a char8_t specialization for this yet.
deletable_facet<std::codecvt<char16_t, char8_t, std::mbstate_t>> conv;

template <typename T>
concept is_specialized = std::is_same_v<char16_t, typename T::intern_type>;
static_assert(is_specialized<std::codecvt<char16_t, char8_t, std::mbstate_t>>);

void i2c::strs::convstr(char const* inp, char16_t* outp, int sz) {
    std::mbstate_t state;
    char8_t const* from_next;
    char16_t* to_next;
    conv.in(state, reinterpret_cast<char8_t const*>(inp), reinterpret_cast<char8_t const*>(inp + sz), from_next, outp, outp + sz, to_next);
}
std::size_t i2c::strs::convstr(char16_t const* inp, char* outp, int isz, int osz) {
    std::mbstate_t state;
    char16_t const* from_next;
    char8_t* to_next;
    auto conv_out = conv.out(state, inp, inp + isz, from_next, reinterpret_cast<char8_t*>(outp), reinterpret_cast<char8_t*>(outp + osz), to_next);
    if (conv_out != std::codecvt_base::ok) {
        throw conv_out;
    }
    return static_cast<size_t>(to_next - reinterpret_cast<char8_t*>(outp));
}

Il2CppString* i2c::strs::alloc_str(std::string_view str) {
    functions::initialize();
    if (str.data() == nullptr) {
        return functions::string_new_len("", 0);
    }
    return functions::string_new_len(str.data(), str.size());
}
Il2CppString* i2c::strs::alloc_str(std::u16string_view str) {
    functions::initialize();
    if (str.data() == nullptr) {
        return functions::string_new_len("", 0);
    }
    return functions::string_new_utf16((Il2CppChar const*) str.data(), str.size());
}

static Il2CppString* create_string(int length) {
    // Why run this method here instead of using string_new_len?
    return i2c::run_method<Il2CppString*>(i2c::class_of<Il2CppString*>(), "CreateString", Il2CppChar('\0'), length);
}

Il2CppString* i2c::strs::strappend(std::string_view const lhs, Il2CppString const* rhs) noexcept {
    if (rhs) {
        int full_length = rhs->length + lhs.size();
        auto result = create_string(full_length);
        convstr(lhs.data(), result->chars, lhs.size());
        auto past_first_string = result->chars + lhs.size();
        memcpy(past_first_string, rhs->chars, rhs->length * sizeof(Il2CppChar));
        return result;
    } else {
        return alloc_str(lhs);
    }
}
Il2CppString* i2c::strs::strappend(std::u16string_view const lhs, Il2CppString const* rhs) noexcept {
    if (rhs) {
        int full_length = rhs->length + lhs.size();
        auto result = create_string(full_length);
        static_assert(sizeof(Il2CppChar) == sizeof(*lhs.data()));
        memcpy(result->chars, lhs.data(), lhs.size() * sizeof(*lhs.data()));
        auto past_first_string = result->chars + lhs.size();
        memcpy(past_first_string, rhs->chars, rhs->length * sizeof(Il2CppChar));
        return result;
    } else {
        return alloc_str(lhs);
    }
}
Il2CppString* i2c::strs::strappend(Il2CppString const* lhs, std::string_view const rhs) noexcept {
    if (lhs) {
        int full_length = lhs->length + rhs.size();
        auto result = create_string(full_length);
        memcpy(result->chars, lhs->chars, lhs->length * sizeof(Il2CppChar));
        auto past_first_string = result->chars + lhs->length;
        convstr(rhs.data(), past_first_string, rhs.size());
        return result;
    } else {
        return alloc_str(rhs);
    }
}
Il2CppString* i2c::strs::strappend(Il2CppString const* lhs, std::u16string_view const rhs) noexcept {
    if (lhs) {
        int full_length = lhs->length + rhs.size();
        auto result = create_string(full_length);
        memcpy(result->chars, lhs->chars, lhs->length * sizeof(Il2CppChar));
        auto past_first_string = result->chars + lhs->length;
        static_assert(sizeof(Il2CppChar) == sizeof(*rhs.data()));
        memcpy(past_first_string, rhs.data(), rhs.size() * sizeof(*rhs.data()));
        return result;
    } else {
        return alloc_str(rhs);
    }
}
Il2CppString* i2c::strs::strappend(Il2CppString const* lhs, Il2CppString const* rhs) noexcept {
    if (!lhs && !rhs) {
        return nullptr;
    }
    functions::initialize();
    if (!lhs && rhs) {
        return functions::string_new_utf16(rhs->chars, rhs->length);
    } else if (lhs && !rhs) {
        return functions::string_new_utf16(lhs->chars, lhs->length);
    } else {
        size_t full_length = lhs->length + rhs->length;
        auto result = create_string(full_length);
        memcpy(result->chars, lhs->chars, lhs->length * sizeof(Il2CppChar));
        auto past_first_string = result->chars + lhs->length;
        memcpy(past_first_string, rhs->chars, rhs->length * sizeof(*rhs->chars));
        return result;
    }
}

struct i2cstr_view {
    constexpr i2cstr_view(Il2CppString const* str) : val(str) {}

    constexpr Il2CppChar const* data() const { return val->chars; }
    constexpr size_t size() const { return static_cast<size_t>(val->length); }

    Il2CppString const* val;
};

static inline bool strcomp_impl(Il2CppString const* lhs, auto const& rhs) noexcept {
    if (!lhs || static_cast<size_t>(lhs->length) != rhs.size()) {
        return false;
    }

    Il2CppChar const* first = lhs->chars;
    auto const* second = rhs.data();
    Il2CppChar const* first_end = first + lhs->length;
    auto const* second_end = second + (int) rhs.size();

    while (first != first_end && second != second_end) {
        if (*first != *second) {
            return false;
        }
        first++;
        second++;
    }

    return first == first_end && second == second_end;
}

bool i2c::strs::strcomp(Il2CppString const* lhs, std::string_view const rhs) noexcept {
    return strcomp_impl(lhs, rhs);
}
bool i2c::strs::strcomp(Il2CppString const* lhs, std::u16string_view const rhs) noexcept {
    return strcomp_impl(lhs, rhs);
}
bool i2c::strs::strcomp(Il2CppString const* lhs, Il2CppString const* rhs) noexcept {
    if (lhs == rhs) {
        return true;
    } else if (!rhs) {
        return false;
    }
    return strcomp_impl(lhs, i2cstr_view(rhs));
}

static inline bool strless_impl(Il2CppString const* lhs, auto const& rhs) noexcept {
    if (!lhs) {
        return true;
    }

    Il2CppChar const* first = lhs->chars;
    auto const* second = rhs.data();
    Il2CppChar const* first_end = lhs->chars + lhs->length;
    auto const* second_end = rhs.data() + rhs.size();

    while (first != first_end && second != second_end) {
        if (*first != *second) {
            return *first < *second;
        }
        first++;
        second++;
    }
    // if we got here, and second is not second end, we had a shorter first, so it should be true
    // if second is the end, we are longer, so it should be false
    return second != second_end;
}

bool i2c::strs::strless(Il2CppString const* lhs, std::string_view const rhs) noexcept {
    return strless_impl(lhs, rhs);
}
bool i2c::strs::strless(Il2CppString const* lhs, std::u16string_view const rhs) noexcept {
    return strless_impl(lhs, rhs);
}
bool i2c::strs::strless(Il2CppString const* lhs, Il2CppString const* rhs) noexcept {
    if (!rhs) {
        return false;
    }
    return strless_impl(lhs, i2cstr_view(rhs));
}

bool strstart_impl(Il2CppString const* lhs, auto const& rhs) noexcept {
    if (!lhs || static_cast<size_t>(lhs->length) < rhs.size()) {
        return false;
    }

    Il2CppChar const* first = lhs->chars;
    auto const* second = rhs.data();
    auto const* second_end = second + rhs.size();

    while (second != second_end) {
        if (*first != *second) {
            return false;
        }
        first++;
        second++;
    }
    // if we got through the entire string it was all equal, return true
    return true;
}

bool i2c::strs::strstart(Il2CppString const* lhs, std::string_view const rhs) noexcept {
    return strstart_impl(lhs, rhs);
}
bool i2c::strs::strstart(Il2CppString const* lhs, std::u16string_view const rhs) noexcept {
    return strstart_impl(lhs, rhs);
}
bool i2c::strs::strstart(Il2CppString const* lhs, Il2CppString const* rhs) noexcept {
    if (!rhs) {
        return false;
    }
    return strstart_impl(lhs, i2cstr_view(rhs));
}

static inline bool strend_impl(Il2CppString const* lhs, auto const& rhs) noexcept {
    if (!lhs || static_cast<size_t>(lhs->length) < rhs.size()) {
        return false;
    }

    Il2CppChar const* first = lhs->chars + lhs->length - 1;
    auto const* second_begin = rhs.data() - 1;
    auto const* second = second_begin + rhs.size();

    while (second != second_begin) {
        if (*first == *second) {
            return false;
        }
        first--;
        second--;
    }
    // if we got through the entire string it was all equal, return true
    return true;
}

bool i2c::strs::strend(Il2CppString const* lhs, std::string_view const rhs) noexcept {
    return strend_impl(lhs, rhs);
}
bool i2c::strs::strend(Il2CppString const* lhs, std::u16string_view const rhs) noexcept {
    return strend_impl(lhs, rhs);
}
bool i2c::strs::strend(Il2CppString const* lhs, Il2CppString const* rhs) noexcept {
    if (!rhs) {
        return false;
    }
    return strend_impl(lhs, i2cstr_view(rhs));
}
