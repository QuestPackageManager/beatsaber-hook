#include "hooking2.hpp"

#include "members.hpp"
#include "tests.hpp"

MAKE_FLAMINGO_HOOK(queue_flamingo_test, ({"System.Collections", "Queue"}, "GetElement", true), void*, void* self, int i) {
    LOG_OK("Flamingo hook run, self -> {} | item -> {} | orig -> {}", fmt::ptr(self), i, queue_flamingo_test(self, i));
    return reinterpret_cast<void*>(uintptr_t(15));
}

TEST(flamingo_priority_hook) {
    LOG_OK("Starting flamingo priority hook test");

    try {
        auto queue = i2c::new_ctor({"System.Collections", "Queue"});
        i2c::run_method(queue, "Enqueue", static_cast<Il2CppObject*>(nullptr));

        modloader::ModInfo other_mod{ "other-mod", "1.0.0", 0 };

        // Install as a namespaced hook with an explicit priority, tracked via a handle rather than on the hook struct.
        auto handle = FLAMINGO_HOOK(i2c::logger, "bs-hooks-tests", queue_flamingo_test).after(other_mod).final().install();
        LOG_OK("Flamingo hook installed");

        auto element = i2c::run_method<Il2CppObject*>(queue, "GetElement", 0);
        LOG_OK("Queue GetElement after flamingo hook install -> {}", fmt::ptr(element));

        // Uninstall, getting back a builder that can be used to reinstall with the same (or a new) priority.
        auto builder = handle.uninstall().value();
        LOG_OK("Flamingo hook uninstalled");

        element = i2c::run_method<Il2CppObject*>(queue, "GetElement", 0);
        LOG_OK("Queue GetElement after flamingo hook uninstall -> {}", fmt::ptr(element));

        handle = builder.before("some-other-mod").install();
        LOG_OK("Flamingo hook reinstalled");

        element = i2c::run_method<Il2CppObject*>(queue, "GetElement", 0);
        LOG_OK("Queue GetElement after flamingo hook reinstall -> {}", fmt::ptr(element));

        handle.uninstall().value();
    } catch (std::exception const& e) {
        LOG_FAIL("Error during flamingo priority hook test: {}", e.what());
        return;
    }

    LOG_OK("Flamingo priority hook test complete");
}
