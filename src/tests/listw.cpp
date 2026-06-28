#include "listw.hpp"

#include "tests.hpp"

static_assert(i2c::type_check::full_class<System::Collections::Generic::List_1<int>*>);
static_assert(i2c::type_check::ptr_ref_type<System::Collections::Generic::List_1<int>*>);

static_assert(i2c::type_check::full_class<ListW<int>>);
static_assert(i2c::type_check::wrapper_ref_type<ListW<int>>);

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

    } catch (std::exception const& e) {
        LOG_FAIL("ListW test failed: {}", e.what());
    }
}
