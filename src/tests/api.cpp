#include "api.hpp"

#include "tests.hpp"

// Assembly enumeration and presence checks
TEST(assembly_enumeration) {
    LOG_OK("Starting assembly enumeration test");

    i2c::functions::initialize();
    auto* domain = i2c::functions::domain_get();
    if (!domain) {
        LOG_FAIL("il2cpp domain_get returned null");
        return;
    }

    size_t asm_count = 0;
    auto** assemblies = i2c::functions::domain_get_assemblies(domain, &asm_count);
    if (!assemblies) {
        LOG_FAIL("domain_get_assemblies returned null");
        return;
    }

    LOG_OK("Found {} assemblies", asm_count);
    std::vector<std::string> names;
    names.reserve(asm_count);
    for (size_t i = 0; i < asm_count; ++i) {
        auto* asm_ptr = assemblies[i];
        if (!asm_ptr) {
            continue;
        }
        auto* img = i2c::functions::assembly_get_image(asm_ptr);
        char const* a_name = img ? img->name : nullptr;
        if (a_name) {
            names.emplace_back(a_name);
            LOG_OK("Assembly[{}] -> {}", i, a_name);
        } else {
            LOG_FAIL("Assembly[{}] had null image/name", i);
        }
    }

    // Common assemblies to look for (these are best-effort; some runtimes differ)
    char const* common[] = {"Assembly-CSharp", "mscorlib", "System", "UnityEngine.CoreModule", "Assembly-CSharp-firstpass"};
    for (auto const& want : common) {
        bool found = false;
        for (auto const& n : names) {
            if (n == want) {
                found = true;
                break;
            }
        }
        if (found) {
            LOG_OK("Found expected assembly: {}", want);
        } else {
            LOG_FAIL("Expected assembly not found (may be okay): {}", want);
        }
    }

    // Ensure we found at least one assembly (sanity)
    if (names.empty()) {
        LOG_FAIL("No assembly names were discovered (unexpected)");
    } else {
        LOG_OK("Assembly enumeration completed successfully ({} entries)", names.size());
    }
}

// resolve_icall must never throw, even for a nonexistent icall - failures are reported through the
// returned i2c::result instead (icall resolution commonly runs during static initialization, where an
// uncaught exception would crash the whole mod at load time).
TEST(resolve_icall_never_throws) {
    constexpr char const* bogus_icall = "This::Icall::Does::Not::Exist";

    bool threw = false;
    i2c::result<function_ptr_t<void>> res(nullptr);
    try {
        res = i2c::resolve_icall<void>(bogus_icall);
    } catch (...) {
        threw = true;
    }
    if (threw) {
        LOG_FAIL("resolve_icall threw for a nonexistent icall - it must return an error result instead");
    } else if (!res.has_value()) {
        LOG_OK("resolve_icall returned an error result instead of throwing: {}", res.error());
    } else {
        LOG_FAIL("resolve_icall unexpectedly resolved a nonexistent icall");
    }

    // Sanity check that a real, long-standing icall still resolves successfully.
    constexpr char const* real_icall = "UnityEngine.Time::get_realtimeSinceStartup";
    auto real_res = i2c::resolve_icall<float>(real_icall);
    if (real_res.has_value()) {
        LOG_OK("resolve_icall resolved a real icall successfully");
    } else {
        LOG_FAIL("resolve_icall failed to resolve a real icall (may be okay if it was renamed/stripped): {}", real_res.error());
    }
}
