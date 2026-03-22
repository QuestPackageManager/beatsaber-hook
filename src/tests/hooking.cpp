#include "hooking.hpp"

#include "members.hpp"
#include "tests.hpp"

MAKE_HOOK(test1, ({"System.Collections", "Queue"}, "Contains", true), bool, void* self, void* item) {
    LOG_OK("Hook run, self -> {} | item -> {} | orig -> {}", fmt::ptr(self), fmt::ptr(item), test1(self, item));
    return true;
}

TEST(queue_hook) {
    LOG_OK("Starting hook test");

    try {
        auto queue = i2c::new_ctor({"System.Collections", "Queue"});
        LOG_OK("Created queue -> {}", fmt::ptr(queue));

        bool contains = i2c::run_method<bool>(queue, "Contains", static_cast<Il2CppObject*>(nullptr));
        LOG_OK("Queue contains before hook -> {}", contains);

        INSTALL_HOOK(i2c::logger, test1);
        LOG_OK("Hook installed");

        contains = i2c::run_method<bool>(queue, "Contains", static_cast<Il2CppObject*>(nullptr));
        LOG_OK("Queue contains after hook install -> {}", contains);

        UNINSTALL_HOOK(i2c::logger, test1);
        LOG_OK("Hook uninstalled");

        contains = i2c::run_method<bool>(queue, "Contains", static_cast<Il2CppObject*>(nullptr));
        LOG_OK("Queue contains after hook uninstall -> {}", contains);
    } catch (std::exception const& e) {
        LOG_FAIL("Error during hook test: {}", e.what());
        return;
    }

    LOG_OK("Hook test complete");
}

// Below only test for compilation

// Test match hook
static int foo(bool) {
    return 0;
}

template <>
struct i2c::metadata_getter<&foo> {
    static MethodInfo const* method_info() { return nullptr; }
    static constexpr size_t size = 24;
    static constexpr uintptr_t addrs = 0x1234;
};

MAKE_HOOK_MATCH(test2, &foo, int, bool) {
    return 1;
}

// Test match hook with static overloads
static int bar(bool) {
    return 0;
}
static int bar(bool b, float) {
    return bar(b);
}

template <>
struct i2c::metadata_getter<static_cast<function_ptr_t<int, bool, float>>(&bar)> {
    static MethodInfo const* method_info() { return nullptr; }
    static constexpr size_t size = 20;
    static constexpr uintptr_t addrs = 0x5678;
};

MAKE_HOOK_MATCH(test3, &bar, int, bool, float) {
    return 1;
}

// Test match hook with instance overloads
struct struct_a {
    int foo(int i) { return foo(i, false, i); }
    int foo(int, bool, int) { return 0; }
};

template <>
struct i2c::metadata_getter<static_cast<method_ptr_t<struct_a, int, int>>(&struct_a::foo)> {
    static MethodInfo const* method_info() { return nullptr; }
    static constexpr size_t size = 60;
    static constexpr uintptr_t addrs = 0x999;
};

MAKE_HOOK_MATCH(test4, &struct_a::foo, int, struct_a*, int) {
    return 1;
}
