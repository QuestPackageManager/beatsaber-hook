#include "hooking.hpp"

#include "members.hpp"
#include "tests.hpp"

MAKE_HOOK(test1, ({"System.Collections", "Queue"}, "GetElement", true), void*, void* self, int i) {
    LOG_OK("Hook run, self -> {} | item -> {} | orig -> {}", fmt::ptr(self), i, test1(self, i));
    return reinterpret_cast<void*>(uintptr_t(5));
}

TEST(queue_hook) {
    LOG_OK("Starting hook test");

    try {
        auto queue = i2c::new_ctor({"System.Collections", "Queue"});
        LOG_OK("Created queue -> {}", fmt::ptr(queue));

        i2c::run_method(queue, "Enqueue", static_cast<Il2CppObject*>(nullptr));
        Il2CppObject* element = i2c::run_method<Il2CppObject*>(queue, "GetElement", 0);
        LOG_OK("Queue GetElement before hook -> {}", fmt::ptr(element));

        INSTALL_HOOK(i2c::logger, test1);
        LOG_OK("Hook installed");

        element = i2c::run_method<Il2CppObject*>(queue, "GetElement", 0);
        LOG_OK("Queue GetElement after hook install -> {}", fmt::ptr(element));

        UNINSTALL_HOOK(i2c::logger, test1);
        LOG_OK("Hook uninstalled");

        element = i2c::run_method<Il2CppObject*>(queue, "GetElement", 0);
        LOG_OK("Queue GetElement after hook uninstall -> {}", fmt::ptr(element));
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
