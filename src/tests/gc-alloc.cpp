#include "gc-alloc.hpp"
#include "tests.hpp"

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
