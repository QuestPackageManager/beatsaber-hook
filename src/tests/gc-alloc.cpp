#include "gc-alloc.hpp"
#include "safeptr.hpp"
#include "tests.hpp"
#include "threading.hpp"

#include <array>
#include <ranges>
#include <vector>

static_assert(std::allocator_traits<i2c::gc_allocator<int>>::is_always_equal::value);
static_assert(std::is_same_v<std::allocator_traits<i2c::gc_allocator<int>>::rebind_alloc<void*>, i2c::gc_allocator<void*>>);

TEST(gc_allocator_vector_and_bounds) {
    i2c::functions::initialize();
    std::vector<int, i2c::gc_allocator<int>> values{7, 13, 29};
    values.reserve(16);
    values.push_back(41);
    if (!std::ranges::equal(values, std::array{7, 13, 29, 41})) {
        LOG_FAIL("GC allocator vector lost values during growth");
        return;
    }

    i2c::gc_allocator<int> const allocator;
    auto* empty = allocator.allocate(0);
    allocator.deallocate(empty, 0);
    if (empty != nullptr) {
        LOG_FAIL("GC allocator did not return null for an empty allocation");
        return;
    }

    try {
        auto* invalid = allocator.allocate(std::numeric_limits<std::size_t>::max() / sizeof(int) + 1);
        allocator.deallocate(invalid, 0);
        LOG_FAIL("GC allocator accepted an overflowing element count");
        return;
    } catch (std::bad_array_new_length const&) {
        LOG_OK("GC allocator vector growth, empty allocation and overflow rejection passed");
    }
}

TEST(gc_allocator_rejects_disabled_gc) {
    using namespace i2c::functions;
    initialize();
    if (gc_is_disabled()) {
        LOG_FAIL("GC must initially be enabled for the allocator state test");
        return;
    }

    i2c::gc_allocator<int> const allocator;
    auto* existing = allocator.allocate(1);
    i2c::on_scope_exit release([&] { allocator.deallocate(existing, 1); });
    bool rejected = false;
    {
        gc_disable();
        i2c::on_scope_exit enable([] { gc_enable(); });
        try {
            auto* unexpected = allocator.allocate(1);
            allocator.deallocate(unexpected, 1);
        } catch (std::bad_alloc const&) {
            rejected = true;
        }
        allocator.deallocate(existing, 1);
        existing = nullptr;
    }

    if (rejected) LOG_OK("GC allocator rejected disabled GC and released an existing block");
    else LOG_FAIL("GC allocator allocated while GC was disabled");
}

TEST(gc_allocator_clear_and_erase) {
    using namespace i2c::functions;
    initialize();
    std::vector<Il2CppObject*, i2c::gc_allocator<Il2CppObject*>> values;
    std::array<Il2CppGCHandle, 2> weak{};
    i2c::on_scope_exit release([&] { for (auto handle : weak) gchandle_free(handle); });

    // Exit the allocating thread so stale stack pointers cannot keep objects alive.
    il2cpp_thread([&] {
        values.reserve(8);
        for (auto& handle : weak) {
            values.push_back(object_new(defaults->object_class));
            handle = gchandle_new_weakref(values.back(), false);
        }
    }).join();
    auto* buffer = values.data();
    auto capacity = values.capacity();
    auto has_live_objects = [&](std::size_t count) {
        for (int i = 0; i < 3; ++i) il2cpp_thread([] { gc_collect(0); }).join();
        bool matches = true;
        il2cpp_thread([&] {
            for (std::size_t i = 0; i < weak.size(); ++i)
                matches &= (gchandle_get_target(weak[i]) != nullptr) == (i < count);
        }).join();
        return matches && values.size() == count && values.data() == buffer && values.capacity() == capacity;
    };

    if (!has_live_objects(2)) { LOG_FAIL("Vector did not retain both managed objects"); return; }
    LOG_OK("Vector retains both managed objects before removal");
    values.erase(values.end() - 1);
    if (has_live_objects(1)) LOG_OK("erase releases the removed object and retains the survivor without reallocating");
    else LOG_FAIL("erase did not release only the removed object with capacity retained");
    values.clear();
    if (has_live_objects(0)) LOG_OK("clear releases the remaining object without reallocating");
    else LOG_FAIL("clear did not release the remaining object with capacity retained");
}

TEST(gc_allocator_clear_with_safeptr) {
    using namespace i2c::functions;
    initialize();
    std::vector<Il2CppObject*, i2c::gc_allocator<Il2CppObject*>> values;
    safe_ptr<Il2CppObject*> keep;
    Il2CppGCHandle weak{};
    i2c::on_scope_exit release([&] { gchandle_free(weak); });

    // Exit these threads so stale stack pointers cannot keep the object alive.
    il2cpp_thread([&] {
        values.push_back(object_new(defaults->object_class));
        keep.emplace(values.back());
        weak = gchandle_new_weakref(values.back(), false);
        values.clear();
    }).join();
    auto is_alive = [&] {
        for (int i = 0; i < 3; ++i) il2cpp_thread([] { gc_collect(0); }).join();
        bool alive = false;
        il2cpp_thread([&] { alive = gchandle_get_target(weak) != nullptr; }).join();
        return alive;
    };

    if (is_alive()) LOG_OK("safe_ptr keeps the object alive after vector clear");
    else LOG_FAIL("Object was collected despite safe_ptr after vector clear");
    il2cpp_thread([&] { keep.clear(); }).join();
    if (!is_alive()) LOG_OK("Object is collectible after releasing the remaining safe_ptr");
    else LOG_FAIL("Object survived after releasing the remaining safe_ptr");
}
