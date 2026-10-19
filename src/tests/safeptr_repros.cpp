#include "safeptr.hpp"
#include "tests.hpp"
#include "threading.hpp"

#include <barrier>
#include <mutex>

namespace {
    // Tests run serially in late_load. Observe the real fixed allocator without reading freed
    // storage or exposing safe_ptr's private members. Worker-thread observations are synchronized.
    struct fixed_gc_probe {
        struct allocation {
            void* address;
            size_t frees = 0;
        };

        inline static fixed_gc_probe* active = nullptr;
        decltype(i2c::functions::gc_alloc_fixed) original_alloc;
        decltype(i2c::functions::gc_free_fixed) original_free;
        std::mutex mutex;
        std::vector<allocation> allocations;

        fixed_gc_probe() {
            i2c::functions::initialize();
            original_alloc = i2c::functions::gc_alloc_fixed;
            original_free = i2c::functions::gc_free_fixed;
            active = this;
            i2c::functions::gc_alloc_fixed = allocate;
            i2c::functions::gc_free_fixed = free;
        }

        ~fixed_gc_probe() {
            i2c::functions::gc_alloc_fixed = original_alloc;
            i2c::functions::gc_free_fixed = original_free;
            active = nullptr;
            // All tested owners have left scope. Clean up leaks after reporting them so a
            // failing regression doesn't leave fixed roots behind for the rest of the suite.
            for (auto const& entry : allocations) {
                if (entry.frees == 0) {
                    LOG_FAIL("Fixed GC wrapper was never freed: {}", fmt::ptr(entry.address));
                    original_free(entry.address);
                }
            }
        }

        static void* allocate(size_t size) {
            auto* result = active->original_alloc(size);
            std::lock_guard lock(active->mutex);
            active->allocations.push_back({result});
            return result;
        }

        static void free(void* address) {
            {
                std::lock_guard lock(active->mutex);
                // An address may be reused by a later allocation; match the newest record.
                for (auto it = active->allocations.rbegin(); it != active->allocations.rend(); ++it) {
                    if (it->address == address) {
                        if (++it->frees != 1) {
                            LOG_FAIL("Fixed GC wrapper was freed more than once: {}", fmt::ptr(address));
                            return;
                        }
                        break;
                    }
                }
            }
            active->original_free(address);
        }

        size_t last() {
            std::lock_guard lock(mutex);
            return allocations.size() - 1;
        }

        allocation get(size_t index) {
            std::lock_guard lock(mutex);
            return allocations.at(index);
        }
    };

    __attribute__((noinline, optnone)) void* convert_without_optimization(safe_ptr<Il2CppObject*> const& ptr) {
        return ptr.convert();
    }
}

TEST(safeptr_copy_assignment_releases_wrapper) {
    fixed_gc_probe probe;
    Il2CppObject first{}, second{};
    safe_ptr<Il2CppObject*> source(&first);
    auto source_id = probe.last();
    safe_ptr<Il2CppObject*> destination(&second);
    auto overwritten_id = probe.last();

    destination = source;
    auto& self = source;
    source = self;
    destination = source;  // Assigning owners of the same wrapper must preserve both references.
    if (destination.ptr() != &first || source.ptr() != &first || probe.get(overwritten_id).frees != 1 || probe.get(source_id).frees != 0 ||
        i2c::detail::get_count(probe.get(source_id).address) != 2) {
        LOG_FAIL("Copy/self/shared assignment did not preserve owners and free the overwritten wrapper exactly once");
        return;
    }

    source.clear();
    if (probe.get(source_id).frees != 0 || destination.ptr() != &first) {
        LOG_FAIL("Clearing a non-final owner freed the shared wrapper");
        return;
    }
    destination.clear();
    destination.clear();
    if (probe.get(source_id).frees != 1 || i2c::detail::get_count(probe.get(source_id).address) != 0) {
        LOG_FAIL("Final/repeated clear did not free the wrapper exactly once and erase its count");
        return;
    }
    LOG_OK("Copy/self/shared assignment and final/repeated clear preserve ownership without leaks");
}

TEST(safeptr_rvalue_assignment_releases_wrapper) {
    fixed_gc_probe probe;
    Il2CppObject first{}, second{};
    safe_ptr<Il2CppObject*> source(&first);
    auto source_id = probe.last();
    safe_ptr<Il2CppObject*> destination(&second);
    auto overwritten_id = probe.last();

    destination = std::move(source);
    // Rvalue assignment still falls back to copying. It must release the overwritten
    // wrapper without imposing new move semantics on the existing API.
    if (source.ptr() != &first || destination.ptr() != &first || probe.get(overwritten_id).frees != 1 ||
        i2c::detail::get_count(probe.get(source_id).address) != 2) {
        LOG_FAIL("Rvalue assignment failed to preserve shared ownership or free the overwritten wrapper");
        return;
    }
    auto& self = destination;
    destination = std::move(self);
    source.clear();
    if (destination.ptr() != &first || probe.get(source_id).frees != 0 || i2c::detail::get_count(probe.get(source_id).address) != 1) {
        LOG_FAIL("Rvalue self-assignment or clearing another owner corrupted ownership");
        return;
    }
    destination = std::move(source);  // Assigning an empty owner must release the destination.
    if (destination.convert() || probe.get(source_id).frees != 1 || i2c::detail::get_count(probe.get(source_id).address) != 0) {
        LOG_FAIL("Assigning an empty rvalue did not release the destination exactly once");
        return;
    }
    LOG_OK("Rvalue/self/empty assignment preserves existing copy semantics without leaks");
}

TEST(safeptr_concurrent_clear_releases_wrapper) {
    fixed_gc_probe probe;
    Il2CppObject instance{};
    constexpr size_t iterations = 256;
    std::vector<safe_ptr<Il2CppObject*>> first, second;
    std::vector<size_t> allocation_ids;
    first.reserve(iterations);
    second.reserve(iterations);
    for (size_t i = 0; i < iterations; ++i) {
        first.emplace_back(&instance);
        allocation_ids.push_back(probe.last());
        second.push_back(first.back());
    }

    std::barrier start(2);
    auto clear = [&](auto& owners) {
        for (auto& owner : owners) {
            start.arrive_and_wait();
            owner.clear();
        }
    };
    il2cpp_thread first_clear([&] { clear(first); });
    il2cpp_thread second_clear([&] { clear(second); });
    first_clear.join();
    second_clear.join();

    for (auto id : allocation_ids) {
        auto entry = probe.get(id);
        if (entry.frees != 1 || i2c::detail::get_count(entry.address) != 0) {
            LOG_FAIL("Concurrent clear freed a wrapper {} times or left a reference count", entry.frees);
            return;
        }
    }
    LOG_OK("Concurrent clear freed all {} shared wrappers exactly once", iterations);
}

TEST(safeptr_empty_conversion) {
    Il2CppObject instance{};
    safe_ptr<Il2CppObject*> empty;
    safe_ptr<Il2CppObject*> null_value(static_cast<Il2CppObject*>(nullptr));
    safe_ptr<Il2CppObject*> source(&instance);
    safe_ptr<Il2CppObject*> moved(std::move(source));
    if (convert_without_optimization(empty) || convert_without_optimization(null_value) || convert_without_optimization(source) ||
        convert_without_optimization(moved) != &instance) {
        LOG_FAIL("Default/null/moved-from conversion did not preserve null and live pointer values");
        return;
    }
    moved.clear();
    if (convert_without_optimization(moved)) {
        LOG_FAIL("Cleared safe_ptr did not convert to null");
        return;
    }
    LOG_OK("Default, null, moved-from, and cleared safe_ptr conversion is null-safe");
}

TEST(countptr_atomic_final_release) {
    Il2CppObject instance{};
    i2c::detail::count_ptr<Il2CppObject> first(&instance), second(first);
    std::barrier start(2);
    Il2CppObject* first_last = nullptr;
    Il2CppObject* second_last = nullptr;
    std::thread first_release([&] {
        start.arrive_and_wait();
        first_last = first.release_last();
    });
    std::thread second_release([&] {
        start.arrive_and_wait();
        second_last = second.release_last();
    });
    first_release.join();
    second_release.join();
    if ((first_last == &instance) == (second_last == &instance) || first.get() || second.get() || i2c::detail::get_count(&instance) != 0 ||
        first.release_last() || i2c::detail::remove_count(&instance)) {
        LOG_FAIL("Atomic final release did not choose exactly one final owner or mishandled an empty count");
        return;
    }
    LOG_OK("Atomic release selects exactly one final owner and handles empty/unregistered pointers");
}
