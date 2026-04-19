#include "types.hpp"

#include "arrayw.hpp"
#include "find.hpp"
#include "members.hpp"
#include "stringw.hpp"
#include "tests.hpp"

// class_of checks: ensure class_of returns expected non-null classes and types match
TEST(class_of_checks) {
    LOG_OK("Starting class_of checks");

    try {
        // Il2CppObject*
        auto* k_obj = i2c::class_of<Il2CppObject*>();
        auto expected_obj = i2c::find_class({"System", "Object"});
        if (k_obj == expected_obj) {
            LOG_OK("class_of(Il2CppObject*) matches System.Object -> {}", fmt::ptr(k_obj));
        } else {
            LOG_FAIL("class_of(Il2CppObject*) != GetClassFromName('System','Object')");
            LOG_FAIL("class_of -> {} | find_class -> {}", fmt::ptr(k_obj), fmt::ptr(expected_obj));
        }

        // Array
        auto* k_arr = i2c::class_of<Il2CppArray*>();
        auto expected_arr = i2c::find_class({"System", "Array"});
        if (k_arr == expected_arr) {
            LOG_OK("class_of(Il2CppArray*) matches System.Array -> {}", fmt::ptr(k_arr));
        } else {
            LOG_FAIL("class_of(Il2CppArray*) != find_class('System','Array')");
            LOG_FAIL("class_of -> {} | find_class -> {}", fmt::ptr(k_arr), fmt::ptr(expected_arr));
        }

        // int (value type)
        auto* k_int = i2c::class_of<int>();
        auto expected_int = i2c::find_class({"System", "Int32"});
        if (k_int == expected_int) {
            LOG_OK("class_of(int) matches System.Int32 -> {}", fmt::ptr(k_int));
        } else {
            LOG_FAIL("class_of(int) != find_class('System','Int32')");
            LOG_FAIL("class_of -> {} | find_class -> {}", fmt::ptr(k_int), fmt::ptr(expected_int));
        }

        // void (pointer type) — log the resolved class name for diagnostics
        auto* k_voidp = i2c::class_of<void>();
        char const* ns = i2c::functions::class_get_namespace(k_voidp);
        char const* name = i2c::functions::class_get_name(k_voidp);
        LOG_OK("class_of(void) -> {}::{} ({})", ns ? ns : "(null)", name ? name : "(null)", fmt::ptr(k_voidp));
        // should be equal to void
        auto expected_void = i2c::find_class({"System", "Void"});
        if (k_voidp == expected_void) {
            LOG_OK("class_of(void) matches System.Void -> {}", fmt::ptr(k_voidp));
        } else {
            LOG_FAIL("class_of(void) != find_class('System','Void')");
            LOG_FAIL("class_of -> {} | find_class -> {}", fmt::ptr(k_voidp), fmt::ptr(expected_void));
        }

        // Il2CppString* and StringW equality
        auto* k_str_raw = i2c::class_of<Il2CppString*>();
        auto* k_str_w = i2c::class_of<StringW>();
        auto expected_str = i2c::find_class({"System", "String"});
        if (k_str_raw == expected_str && k_str_w == expected_str) {
            LOG_OK("class_of(Il2CppString*) and class_of(StringW) both match System.String -> {}", fmt::ptr(expected_str));
        } else {
            LOG_FAIL("class_of(String) variants do not match System.String");
            LOG_FAIL("Il2CppString* -> {} | StringW -> {} | expected -> {}", fmt::ptr(k_str_raw), fmt::ptr(k_str_w), fmt::ptr(expected_str));
        }

        // ArrayW<T> vs Array<T>* class_of equality check for reference-type elements
        // Ensure that the array wrapper maps to the underlying Il2Cpp array class
        auto* k_arrw_obj = i2c::class_of<ArrayW<Il2CppObject*>>();
        auto* k_arr_raw_obj = i2c::class_of<Array<Il2CppObject*>*>();
        if (k_arrw_obj == k_arr_raw_obj) {
            LOG_OK("class_of(ArrayW<Il2CppObject*>) == class_of(Array<Il2CppObject*>*)");
        } else {
            LOG_FAIL("class_of(ArrayW<Il2CppObject*>) != class_of(Array<Il2CppObject*>*)");
            LOG_FAIL("ArrayW -> {} | raw Array -> {}", fmt::ptr(k_arrw_obj), fmt::ptr(k_arr_raw_obj));
        }

        // Also validate element class is Il2CppObject*
        auto* elem_k = i2c::functions::class_get_element_class(k_arr_raw_obj);
        if (elem_k == i2c::class_of<Il2CppObject*>()) {
            LOG_OK("class_get_element_class(Array<Il2CppObject*>*) == class_of(Il2CppObject*)");
        } else {
            LOG_FAIL("Element class for Array<Il2CppObject*>* is not Il2CppObject* as expected");
            LOG_FAIL("element class -> {} | Il2CppObject* -> {}", fmt::ptr(elem_k), fmt::ptr(i2c::class_of<Il2CppObject*>()));
        }
    } catch (std::exception const& e) {
        LOG_FAIL("class_of checks failed: {}", e.what());
    }
}

TEST(arrays_and_generics) {
    LOG_OK("Starting array & generics checks");

    try {
        // Try creating an array of System.Object
        auto obj_klass = i2c::find_class({"System", "Object"});
        auto* arr = i2c::functions::array_new(obj_klass, 3);
        LOG_OK("Created System.Object[]: {}", fmt::ptr(arr));
    } catch (std::exception const& e) {
        LOG_FAIL("System.Object klass not found, skipping array creation: {}", e.what());
    }

    try {
        // Check for generic List<T> class presence (without instantiation)
        auto list_klass = i2c::find_class({"System.Collections.Generic", "List`1"});
        LOG_OK("Found generic type List`1: {}", fmt::ptr(list_klass));
    } catch (std::exception const& e) {
        LOG_FAIL("Generic List`1 not found in assemblies: {}", e.what());
    }

    // call a generic method - List<int>.Add
    try {
        auto list_klass = i2c::find_class({"System.Collections.Generic", "List`1"});
        auto int_klass = i2c::find_class({"System", "Int32"});

        // Make List<int> generic class
        auto* generic_list_klass = i2c::make_generic(list_klass, {int_klass});
        if (!generic_list_klass) {
            LOG_FAIL("Could not make List<int> generic class");
            return;
        }

        // Create instance of List<int>
        auto list_instance = i2c::new_ctor(generic_list_klass);

        // Call Add(42)
        i2c::run_method<void>(list_instance, "Add", 42);
        LOG_OK("Successfully called List<int>.Add(42)");

        // Read backing field (int) usually named "_size" on many List<T> implementations
        auto size = i2c::get_field<int>(list_instance, "_size");
        LOG_OK("List<int> _size before set -> {}", size);

        // Try setting the _size field to 1 and read back
        i2c::set_field(list_instance, "_size", 1);
        LOG_OK("Set List<int>._size = 1 (success)");
        auto new_size = i2c::get_field<int>(list_instance, "_size");
        LOG_OK("List<int> _size after set -> {}", new_size);
    } catch (std::exception const& e) {
        LOG_FAIL("List<int> test failed: {}", e.what());
    }
}
