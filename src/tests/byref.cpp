#include "byref.hpp"
#include "members.hpp"
#include "valuew.hpp"

#include <bit>

#include "tests.hpp"

// Technically not a valid type due to the lack of a no_arg_class
static_assert(i2c::type_check::has_get<i2c::type_check::no_arg_type<by_ref<int>>>);
static_assert(i2c::type_check::wrapper_ref_type<by_ref<int>>);

static_assert(i2c::type_check::wrapper_ref_type<by_ref<int const>>);
static_assert(std::is_convertible_v<int const&, by_ref<int const>>);
static_assert(std::is_convertible_v<int&&, by_ref<int const>>);
static_assert(std::is_convertible_v<by_ref<int>, by_ref<int const>>);
static_assert(std::is_copy_constructible_v<by_ref<int>>);
static_assert(std::is_copy_constructible_v<by_ref<int const>>);
static_assert(!std::is_convertible_v<int const&, by_ref<int>>);
static_assert(!std::is_convertible_v<int&&, by_ref<int>>);
static_assert(!std::is_convertible_v<by_ref<int const>, by_ref<int>>);
static_assert(std::is_same_v<decltype(*std::declval<by_ref<int const>&>()), int const&>);
static_assert(!std::is_assignable_v<decltype(*std::declval<by_ref<int const>&>()), int>);
static_assert(std::is_same_v<decltype(*std::declval<by_ref<Il2CppObject* const>&>()), Il2CppObject* const&>);

TEST(byref_readonly) {
    int const value = 123;
    by_ref<int const> readonly(value);
    if (readonly.convert() != &value || *readonly != value) {
        LOG_FAIL("Readonly by_ref did not preserve the referent");
        return;
    }
    int mutable_value = 456;
    by_ref<int> writable(mutable_value);
    by_ref<int const> converted(writable);
    *writable = 789;
    if (*converted != 789 || converted.convert() != &mutable_value) {
        LOG_FAIL("Mutable-to-readonly conversion did not preserve the referent");
        return;
    }
    try {
        auto mutable_type = i2c::type_of<by_ref<int>>();
        auto readonly_type = i2c::type_of<by_ref<int const>>();
        auto pointer_type = i2c::type_of<by_ref<Il2CppObject* const>>();
        if (!readonly_type || readonly_type != mutable_type || !readonly_type->byref ||
            pointer_type != i2c::type_of<by_ref<Il2CppObject*>>()) {
            LOG_FAIL("Readonly by_ref metadata must match mutable byref metadata");
            return;
        }
        LOG_OK("Readonly by_ref storage, conversion and metadata");
    } catch (std::exception const& e) {
        LOG_FAIL("Readonly by_ref metadata failed: {}", e.what());
    }
}

// Unity 6.3 exposes these Quaternion parameters as readonly byrefs.
TEST(byref_readonly_quaternion_invocation) {
    using Quaternion = ValueW<16, "UnityEngine", "Quaternion">;
    auto quaternion = [](float x, float y, float z, float w) {
        return Quaternion(std::bit_cast<std::array<std::byte, 16>>(std::array{x, y, z, w}));
    };
    auto dot = [](by_ref<Quaternion const> a, by_ref<Quaternion const> b) {
        return i2c::run_method<float>({"UnityEngine", "Quaternion"}, "Dot", a, b);
    };
    auto normalize = [](by_ref<Quaternion const> value) {
        return i2c::run_method<Quaternion>({"UnityEngine", "Quaternion"}, "Normalize", value);
    };
    try {
        Quaternion const a = quaternion(1, 2, 3, 4);
        auto const before = a.instance;
        // A typed temporary remains alive for this entire call expression.
        if (dot(a, quaternion(5, 6, 7, 8)) != 70.0f || a.instance != before) {
            LOG_FAIL("Quaternion.Dot readonly invocation returned a wrong result or changed its input");
            return;
        }
        auto normalized = normalize(quaternion(0, 0, 0, 2));
        if (normalized.instance != quaternion(0, 0, 0, 1).instance) {
            LOG_FAIL("Quaternion.Normalize readonly invocation returned a wrong result");
            return;
        }
        LOG_OK("Readonly Quaternion lookup and invocation with const and temporary arguments");
    } catch (std::exception const& e) {
        LOG_FAIL("Readonly Quaternion invocation failed: {}", e.what());
    }
}

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
