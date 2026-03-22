#include "members.hpp"

#include "stringw.hpp"
#include "tests.hpp"

TEST(basic_creation_and_methods) {
    LOG_OK("Starting basic creation & method tests");

    // Create a System.Object instance and call ToString()
    try {
        auto obj = i2c::new_ctor({"System", "Object"});
        LOG_OK("Created System.Object: {}", fmt::ptr(obj));
        auto str = i2c::run_method<StringW>(obj, "ToString");
        LOG_OK("Object.ToString() returned object ptr: {}", fmt::ptr(str.convert()));
    } catch (std::exception const& e) {
        LOG_FAIL("Object.ToString() test failed: {}", e.what());
    }

    // String.Concat test
    try {
        StringW s1("Hello ");
        StringW s2("World");

        auto concat = i2c::run_method<StringW>({"System", "String"}, "Concat", s1, s2);
        LOG_OK("String.Concat succeeded, result ptr: {}", fmt::ptr(concat.convert()));
    } catch (std::exception const& e) {
        LOG_FAIL("String.Concat failed: {}", e.what());
    }
}

// Test invoking a C# method that will throw and using exception handling
TEST(runmethodrethrow_on_throwing_method) {
    LOG_OK("Starting exception handling tests (invoke C# method that throws)");

    try {
        // System.Int32.Parse will throw FormatException for non-numeric input
        auto val = i2c::run_method<int>({"System", "Int32"}, "Parse", StringW("notanint"));
        LOG_FAIL("Unexpected success from Int32.Parse, value: {}", val);
    } catch (std::exception const& e) {
        LOG_OK("Caught exception from Int32.Parse: {}", e.what());
    }
}

TEST(get_set_field_on_non_generic) {
    LOG_OK("Starting non-generic get/set field test");

    try {
        // We'll try System.Text.StringBuilder as a common non-generic runtime type with int fields
        auto instance = i2c::new_ctor({"System.Text", "StringBuilder"});

        // Candidate field names (common variants across runtimes)
        char const* field_candidates[] = {"m_MaxCapacity", "m_ChunkLength", "m_capacity", "m_ChunkOffset"};

        for (auto const& f_name : field_candidates) {
            try {
                auto val = i2c::get_field<int>(instance, f_name);
                LOG_OK("Read field '{}' -> {}", f_name, val);
                // Try setting it to val+1 (defensive)
                i2c::set_field<int>(instance, f_name, val + 1);
                LOG_OK("Successfully set field '{}' -> {}", f_name, val + 1);
                auto new_val = i2c::get_field<int>(instance, f_name);
                LOG_OK("Read back field '{}' -> {}", f_name, new_val);
                return;  // done with first successful candidate
            } catch (std::exception const&) {
                // Field not found, try next one
                continue;
            }
        }

        LOG_FAIL("No suitable int field found on StringBuilder from candidates");
    } catch (std::exception const& e) {
        LOG_FAIL("StringBuilder test failed: {}", e.what());
    }
}

// Property getter/setter tests
TEST(property_get_set) {
    LOG_OK("Starting property get/set tests");

    try {
        // Test System.Text.StringBuilder.Length getter/setter
        auto sb = i2c::new_ctor({"System.Text", "StringBuilder"});

        auto len = i2c::get_property<int>(sb, "Length");
        LOG_OK("StringBuilder.Length initial -> {}", len);

        i2c::set_property<int>(sb, "Length", 5);
        LOG_OK("Called set_Length(5) on StringBuilder");
        auto new_len = i2c::get_property<int>(sb, "Length");
        LOG_OK("StringBuilder.Length after set -> {}", new_len);
    } catch (std::exception const& e) {
        LOG_FAIL("StringBuilder property tests failed: {}", e.what());
    }

    // Test List<int>.Count getter after Add
    try {
        auto list_klass = i2c::find_class({"System.Collections.Generic", "List`1"});
        auto int_klass = i2c::find_class({"System", "Int32"});

        auto generic_list_klass = i2c::make_generic(list_klass, {int_klass});
        auto list_instance = i2c::new_ctor(generic_list_klass);

        i2c::run_method<void>(list_instance, "Add", 10);
        LOG_OK("Called List<int>.Add(10)");
        auto cnt = i2c::get_property<int>(list_instance, "Count");
        LOG_OK("List<int>.Count -> {}", cnt);
    } catch (std::exception const& e) {
        LOG_FAIL("List<int> property test failed: {}", e.what());
    }
}
