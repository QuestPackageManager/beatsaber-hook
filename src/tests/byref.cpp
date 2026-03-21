#include "byref.hpp"

#include "tests.hpp"

// Technically not a valid type due to the lack of a no_arg_class
static_assert(i2c::type_check::has_get<i2c::type_check::no_arg_type<by_ref<int>>>);
static_assert(i2c::type_check::wrapper_ref_type<by_ref<int>>);

// ByRef helper tests: exercise type_of and by_ref construction
TEST(byref_helpers) {
    LOG_OK("Starting by_ref helper tests");

    try {
        // Construct a by_ref instance from a local int and validate that it can be created
        int local = 123;
        by_ref<int> const br(local);
        LOG_OK("Constructed by_ref<int> successfully");

        // Confirm that type_of<int>() is non-null as a sanity check
        if (auto t_int = i2c::type_of<int>()) {
            LOG_OK("type_of<int> present {}", fmt::ptr(t_int));
        } else {
            LOG_FAIL("type_of<int> returned null (unexpected)");
        }

        int local2 = 456;
        by_ref<int> br2(local);
        br2 = local2;
    } catch (std::exception const& e) {
        LOG_FAIL("Exception during by_ref tests: {}", e.what());
    } catch (...) {
        LOG_FAIL("Unknown exception during by_ref tests");
    }
}
