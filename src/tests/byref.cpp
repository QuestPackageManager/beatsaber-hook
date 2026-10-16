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

static_assert(std::is_same_v<decltype(std::declval<by_ref<int> const&>().as_const()), by_ref<int const>>);
static_assert(std::is_same_v<decltype(std::declval<by_ref<int const> const&>().as_const()), by_ref<int const>>);
static_assert(std::is_same_v<decltype(std::declval<by_ref<int> const&>().as_mutable()), by_ref<int>>);
static_assert(std::is_same_v<decltype(std::declval<by_ref<int const> const&>().as_mutable()), by_ref<int>>);
static_assert(std::is_same_v<decltype(std::declval<by_ref<int const* const>>().as_mutable()), by_ref<int const*>>);
static_assert(std::is_same_v<decltype(std::declval<by_ref<int const volatile>>().as_mutable()), by_ref<int volatile>>);
static_assert(noexcept(std::declval<by_ref<int> const&>().as_const()));
static_assert(noexcept(std::declval<by_ref<int const> const&>().as_mutable()));
static_assert([] {
    int value = 1;
    by_ref<int> writable(value);
    auto readonly = writable.as_const();
    *readonly.as_mutable() = 2;
    return value == 2 && readonly.ref == &value && readonly.as_mutable().ref == &value;
}());

TEST(byref_constness_helpers) {
    int value = 123;
    by_ref<int> const writable(value);
    auto const readonly = writable.as_const();
    auto mutable_view = readonly.as_mutable();
    *mutable_view = 456;
    if (value != 456 || *readonly != 456 || mutable_view.ref != &value || readonly.ref != &value || writable.as_mutable().ref != &value ||
        readonly.as_const().ref != &value) {
        LOG_FAIL("Byref constness helpers did not preserve shared storage");
        return;
    }

    // Removing const from the pointer slot must preserve const on its pointee.
    int const first = 1;
    int const second = 2;
    int const* pointer = &first;
    by_ref<int const* const> readonly_pointer(pointer);
    *readonly_pointer.as_mutable() = &second;
    if (pointer != &second || *readonly_pointer != &second || first != 1 || second != 2) {
        LOG_FAIL("Mutable byref view did not preserve nested pointer constness");
        return;
    }
    LOG_OK("Explicit byref constness helpers share storage and preserve nested constness");
}

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

TEST(byref_readonly_overload_resolution) {
    using Quaternion = ValueW<16, "UnityEngine", "Quaternion">;
    auto* klass = i2c::class_of<Quaternion>();
    auto* value_type = i2c::type_of<Quaternion>();
    auto* readonly_type = i2c::type_of<by_ref<Quaternion const>>();
    auto* mutable_type = i2c::type_of<by_ref<Quaternion>>();
    auto* readonly_method = i2c::find_method(klass, {"Equals", {}, {readonly_type}});
    auto* value_method = i2c::find_method(klass, {"Equals", {}, {value_type}});
    if (!readonly_method || !value_method || readonly_method == value_method ||
        !readonly_method->parameters[0]->byref || value_method->parameters[0]->byref ||
        i2c::functions::class_from_il2cpp_type(readonly_method->parameters[0]) != klass ||
        i2c::functions::class_from_il2cpp_type(value_method->parameters[0]) != klass) {
        LOG_FAIL("Quaternion.Equals by-value and readonly lookups did not select distinct matching overloads");
        return;
    }
    if (i2c::find_method(klass, {"Equals", {}, {mutable_type}}) != readonly_method ||
        i2c::find_method(klass, {"Equals", {}, {readonly_type}}) != readonly_method ||
        i2c::find_method(klass, {"Equals", {}, {value_type}}) != value_method) {
        LOG_FAIL("Quaternion.Equals cached lookup did not preserve byref status");
        return;
    }
    if (i2c::param_match(value_method, {}, {readonly_type}) != i2c::match::convertible ||
        i2c::param_match(readonly_method, {}, {value_type}) != i2c::match::none ||
        i2c::param_match(value_method, {}, {value_type}) != i2c::match::exact ||
        i2c::param_match(readonly_method, {}, {readonly_type}) != i2c::match::exact ||
        i2c::param_match(readonly_method, {}, {mutable_type}) != i2c::match::exact) {
        LOG_FAIL("Argument matching did not distinguish exact signatures from byref-to-value conversions");
        return;
    }
    auto value_type_copy = *value_method->parameters[0];
    auto readonly_type_copy = *readonly_method->parameters[0];
    if (i2c::param_match(value_method, {}, {&value_type_copy}) != i2c::match::exact ||
        i2c::param_match(readonly_method, {}, {&readonly_type_copy}) != i2c::match::exact) {
        LOG_FAIL("Equivalent type metadata at different addresses did not match exactly");
        return;
    }
    if (!i2c::is_convertible_from(value_type, readonly_type) || !i2c::is_convertible_from(readonly_type, value_type)) {
        LOG_FAIL("Non-argument type conversion unexpectedly requires matching byref status");
        return;
    }

    Quaternion const a(std::bit_cast<std::array<std::byte, 16>>(std::array{1.0f, 2.0f, 3.0f, 4.0f}));
    Quaternion const same = a;
    Quaternion const different(std::bit_cast<std::array<std::byte, 16>>(std::array{1.0f, 2.0f, 3.0f, 5.0f}));
    auto const before = a.instance;
    if (!i2c::run_method<bool>(a, "Equals", same) || i2c::run_method<bool>(a, "Equals", different) ||
        !i2c::run_method<bool>(a, "Equals", by_ref<Quaternion const>(same)) ||
        i2c::run_method<bool>(a, "Equals", by_ref<Quaternion const>(different)) || a.instance != before) {
        LOG_FAIL("Quaternion.Equals overload invocation returned a wrong result or changed its input");
        return;
    }
    LOG_OK("Quaternion.Equals resolves, caches and invokes distinct by-value and readonly overloads");
}

TEST(byref_value_fallback) {
    int value = -7;
    auto* method = i2c::find_method({"System", "Math"}, {"Abs", {}, {i2c::type_of<by_ref<int>>()}});
    if (!method || method->parameters[0]->byref ||
        i2c::param_match(method, {}, {i2c::type_of<by_ref<int>>()}) != i2c::match::convertible) {
        LOG_FAIL("Byref argument did not fall back to the by-value Math.Abs overload");
        return;
    }
    if (i2c::run_method<int>({"System", "Math"}, "Abs", by_ref<int>(value)) != 7 || value != -7) {
        LOG_FAIL("Byref-to-value invocation returned a wrong result or changed its input");
        return;
    }
    LOG_OK("Byref-to-value fallback resolves and invokes Math.Abs without changing its input");
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
