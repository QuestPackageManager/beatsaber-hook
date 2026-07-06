#include "safeptr.hpp"

#include <shared_mutex>

static std::unordered_map<void*, size_t> addr_ref_count;
static std::shared_mutex mutex;

void i2c::detail::add_count(void* addr) {
    std::unique_lock lock(mutex);
    auto itr = addr_ref_count.find(addr);
    if (itr != addr_ref_count.end()) {
        ++itr->second;
    } else {
        addr_ref_count.emplace(addr, 1);
    }
}

void i2c::detail::remove_count(void* addr) {
    std::unique_lock lock(mutex);
    auto itr = addr_ref_count.find(addr);
    if (itr != addr_ref_count.end() && itr->second > 1) {
        --itr->second;
    } else if (itr != addr_ref_count.end()) {
        addr_ref_count.erase(itr);
    }
}

size_t i2c::detail::get_count(void* addr) {
    std::shared_lock lock(mutex);
    auto itr = addr_ref_count.find(addr);
    if (itr != addr_ref_count.end()) {
        return itr->second;
    } else {
        return 0;
    }
}
