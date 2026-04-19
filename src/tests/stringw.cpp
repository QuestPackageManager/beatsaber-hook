#include "stringw.hpp"

#include "tests.hpp"

static_assert(i2c::type_check::full_class<Il2CppString*>);
static_assert(i2c::type_check::ptr_ref_type<Il2CppString*>);

static_assert(i2c::type_check::full_class<StringW>);
static_assert(i2c::type_check::wrapper_ref_type<StringW>);

TEST(stringw) {
    LOG_OK("Starting StringW tests");

    StringW s1("hello");
    StringW s2(" world");
    auto s3 = s1 + s2;
    LOG_OK("s1 -> {} | s2 -> {} | s3 -> {}", s1, s2, s3);

    LOG_OK("s3.starts_with('he') -> {} | s3.ends_with('ld') -> {}", s3.starts_with("he"), s3.ends_with("ld"));

    // concatenation with std::string
    StringW s4 = s3 + std::string("!");
    LOG_OK("s4 -> {}", s4);

    // Test operator+=
    StringW s5 = "test";
    s5 += "ing";
    LOG_OK("s5 after += 'ing' -> {}", s5);

    // Test comparisons
    StringW s6("abc");
    StringW s7("def");
    LOG_OK("s6 == 'abc' -> {}", s6 == "abc");
    LOG_OK("s6 < s7 -> {}", s6 < s7);
    LOG_OK("s7 < s6 -> {}", s7 < s6);

    // Test starts_with and ends_with with different types
    LOG_OK("s3.starts_with(StringW('hel')) -> {}", s3.starts_with(StringW("hel")));
    LOG_OK("s3.ends_with(std::string('ld')) -> {}", s3.ends_with(std::string("ld")));

    // Test operator[]
    LOG_OK("s3[0] -> {}", static_cast<char>(s3[0]));
    LOG_OK("s3[4] -> {}", static_cast<char>(s3[4]));

    // Test iterators
    LOG_OK("Iterating s3:");
    for (auto ch : s3) {
        LOG_OK("  {}", static_cast<char>(ch));
    }

    // Test conversions
    std::string std_str = s3;
    std::u16string u16_str = s3;
    std::u16string_view u16_view = s3;
    LOG_OK("std::string conversion: {}", std_str);
    LOG_OK("std::u16string size: {}", u16_str.size());
    LOG_OK("std::u16string_view size: {}", u16_view.size());

    // Test span operators
    auto span = static_cast<std::span<Il2CppChar>>(s3);
    LOG_OK("span size: {}", span.size());

    // Test const variants
    StringW const const_s("const");
    LOG_OK("const_s == 'const' -> {}", const_s == "const");
    LOG_OK("const_s.starts_with('con') -> {}", const_s.starts_with("con"));
    LOG_OK("const_s.ends_with('st') -> {}", const_s.ends_with("st"));
    LOG_OK("const_s[0] -> {}", static_cast<char>(const_s[0]));

    // Test empty string
    StringW empty("");
    LOG_OK("empty string size: {}", empty ? std::string(empty).size() : 0);

    // Test nullptr
    StringW null_str(nullptr);
    LOG_OK("null_str is null: {}", !null_str);

    // Test assignment
    null_str = empty;
    null_str = const_s;
    null_str = std::move(empty);
    null_str = nullptr;
}
