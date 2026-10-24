#pragma once

#include "api.hpp"

#include <cstddef>
#include <limits>
#include <new>
#include <type_traits>

namespace i2c {
    // STL allocator for scanned, explicitly freed GC storage. Nonempty
    // allocations require an enabled GC; there is no native-heap fallback.
    template <typename T>
    struct gc_allocator {
        using value_type = T;
        using is_always_equal = std::true_type;

        constexpr gc_allocator() noexcept = default;

        template <typename U>
        constexpr gc_allocator(gc_allocator<U> const&) noexcept {}

        template <typename U>
        constexpr bool operator==(gc_allocator<U> const&) const noexcept { return true; }

        [[nodiscard]] T* allocate(std::size_t count) const {
            if (count == 0) return nullptr;
            if (count > std::numeric_limits<std::size_t>::max() / sizeof(T)) throw std::bad_array_new_length();

            functions::initialize();
            if (!functions::gc_alloc_fixed || !functions::gc_free_fixed || !functions::gc_is_disabled ||
                functions::gc_is_disabled()) {
                throw std::bad_alloc();
            }
            auto* memory = static_cast<T*>(functions::gc_alloc_fixed(count * sizeof(T)));
            if (!memory) throw std::bad_alloc();
            return memory;
        }

        void deallocate(T* memory, std::size_t) const noexcept {
            // Existing blocks must still be releasable if GC is disabled later.
            if (memory) functions::gc_free_fixed(memory);
        }
    };
}
