#include "listw.hpp"
#include "stringw.hpp"

#include "tests.hpp"

static_assert(i2c::type_check::full_class<System::Collections::Generic::List_1<int>*>);
static_assert(i2c::type_check::ptr_ref_type<System::Collections::Generic::List_1<int>*>);

static_assert(i2c::type_check::full_class<ListW<int>>);
static_assert(i2c::type_check::wrapper_ref_type<ListW<int>>);

// ListW must not silently reinterpret element types via the view<U>/vector<U> converting constructors
static_assert(!std::is_constructible_v<ListW<int>, i2c::view<void*>>);
static_assert(!std::is_constructible_v<ListW<int>, std::vector<void*>>);
static_assert(!std::is_constructible_v<ListW<int>, ListW<void*>>);
// Arbitrary void* construction is still intentionally allowed (e.g. wrapping reflection results)
static_assert(std::is_constructible_v<ListW<int>, void*>);

// ListW must not silently narrow/reinterpret element types via the view<U>/vector<U> converting constructors
static_assert(!std::is_constructible_v<ListW<float>, std::vector<int>>);
static_assert(!std::is_constructible_v<ListW<float>, i2c::view<int>>);
static_assert(std::is_constructible_v<ListW<int>, std::vector<int>>);
static_assert(std::is_constructible_v<ListW<int>, i2c::view<int>>);
// ListW's converting constructors only ever read from the source, so const spans/vectors/views work too
static_assert(std::is_constructible_v<ListW<int>, std::span<int const>>);
static_assert(std::is_constructible_v<ListW<int>, std::vector<int> const&>);
static_assert(std::is_constructible_v<ListW<int>, i2c::view<int const>>);
// ...but ABI-equivalent wrapper conversions (opted into via i2c::abi_convertible) are still allowed
static_assert(std::is_constructible_v<ListW<StringW>, std::vector<StringW::ptr>>);
static_assert(std::is_constructible_v<ListW<ArrayW<int>>, std::vector<Array<int>*>>);

TEST(listw) {
    LOG_OK("Starting ListW tests");

    try {
        // Create a managed List<int> and wrap it
        auto arr = ListW<int>::New();
        arr.push_back(7);
        arr.push_back(13);
        LOG_OK("Created ListW<int> arr (ptr {})", fmt::ptr(arr.convert()));
        LOG_OK("After construction: size -> {}", arr.size());

        // Test element access
        if (arr.size() >= 2) {
            LOG_OK("Element access: [0] -> {} | [1] -> {}", arr[0], arr[1]);
        }

        // Test finding an element
        auto found = arr.find_if([](int const& v) { return v == 13; });
        LOG_OK("find(13) -> {}", found != arr.end());

        arr.clear();
        LOG_OK("After clear: size -> {}", arr.size());

        // Test push_back
        arr.push_back(1);
        int x = 2;
        arr.push_back(x);
        LOG_OK("After push_back: size -> {}, elements -> {}", arr.size(), arr);

        // Test insert_at
        arr.insert_at(1, 3);
        LOG_OK("After insert_at: size -> {}, elements -> {}", arr.size(), arr);

        // Test const variants
        ListW<int> const const_arr = arr;  // Copy
        LOG_OK("const_arr size -> {}", const_arr.size());
        LOG_OK("const_arr[0] -> {}", const_arr[0]);

        // Test at and try_get
        LOG_OK("arr.at(0) -> {}", arr.at(0));
        auto opt = arr.try_get(0);
        if (opt) {
            LOG_OK("arr.try_get(0) -> {}", opt->get());
        }

        // Test find functions
        auto it = arr.find(1);
        LOG_OK("arr.find(1) != end() -> {}", it != arr.end());

        // Test contains
        LOG_OK("arr.contains(1) -> {}", arr.contains(1));

        // Test index_of
        LOG_OK("arr.index_of(1) -> {}", arr.index_of(1));

        // Test front/back
        LOG_OK("arr.front() -> {}", arr.front());
        LOG_OK("arr.back() -> {}", arr.back());

        // Test copy_to
        std::vector<int> dest(5);
        arr.copy_to(dest, 0);
        LOG_OK("After copy_to, dest -> {}", fmt::join(dest.begin(), dest.end(), ", "));

        // Test to_array
        auto array = arr.to_array();
        LOG_OK("to_array() size -> {}", array.size());

        // Test iterators
        LOG_OK("Iterating arr:");
        for (auto val : arr) {
            LOG_OK("  {}", val);
        }

        // Test ref_to
        auto span = arr.ref_to();
        LOG_OK("arr.ref_to() size -> {}", span.size());

        // Test construction from std::vector
        std::vector<int> vec_src = {21, 22, 23};
        ListW<int> from_vec(vec_src);
        LOG_OK("Constructed ListW<int> from_vec from std::vector size -> {}, elements -> {}", from_vec.size(), from_vec);

        // Test construction from an ABI-equivalent wrapper's underlying pointer type
        std::vector<StringW::ptr> str_vec_src = {static_cast<StringW::ptr>(StringW("foo")), static_cast<StringW::ptr>(StringW("bar"))};
        ListW<StringW> from_str_vec(str_vec_src);
        LOG_OK("Constructed ListW<StringW> from_str_vec from std::vector<StringW::ptr> size -> {}", from_str_vec.size());

        // Test construction from std::span
        std::array<int, 3> span_backing = {31, 32, 33};
        std::span<int> span_src(span_backing);
        ListW<int> from_span(span_src);
        LOG_OK("Constructed ListW<int> from_span from std::span size -> {}, elements -> {}", from_span.size(), from_span);

        // Test construction from a const std::span, const std::vector, and i2c::view -- ListW's converting
        // constructors only ever read from the source, so none of these should require a mutable container.
        std::span<int const> const_span_src(span_backing);
        ListW<int> from_const_span(const_span_src);
        LOG_OK("Constructed ListW<int> from_const_span from std::span<const> size -> {}, elements -> {}", from_const_span.size(), from_const_span);

        std::vector<int> const const_vec_src = {34, 35, 36};
        ListW<int> from_const_vec(const_vec_src);
        LOG_OK("Constructed ListW<int> from_const_vec from std::vector<int> const size -> {}, elements -> {}", from_const_vec.size(), from_const_vec);

        // i2c::view<T> is already read-only (it wraps std::span<T const>), so it's the "const view" type -- there's
        // no separate view<T const> for this purpose (its value_type must match T exactly, so it can't be built
        // from an ordinary container like this).
        i2c::view<int> const_view_src(const_vec_src);
        ListW<int> from_const_view(const_view_src);
        LOG_OK("Constructed ListW<int> from_const_view from i2c::view<int> size -> {}, elements -> {}", from_const_view.size(), from_const_view);

        // Test implicit conversion from std::vector when passed as ListW parameter.
        // Note: this only works for std::vector, not std::span -- ListW's dedicated std::vector<U> constructor
        // is a single user-defined conversion, whereas std::span would need two (span -> i2c::view -> ListW),
        // and implicit conversions only ever apply one.
        auto take_listw = [](ListW<int> l) { return l.size(); };
        LOG_OK("Implicit std::vector -> ListW size -> {}", take_listw(vec_src));

        // Test converting ListW to std::span implicitly
        std::span<int> converted_span = from_vec;
        LOG_OK("Converted ListW to std::span, size -> {} | first -> {}", converted_span.size(), converted_span.front());

        // Test converting const ListW to std::span of const values
        std::span<int const> converted_const_span = const_arr;
        LOG_OK("Converted const ListW to std::span<const>, size -> {}", converted_const_span.size());

    } catch (std::exception const& e) {
        LOG_FAIL("ListW test failed: {}", e.what());
    }
}
