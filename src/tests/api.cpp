#include "api.hpp"

#include "tests.hpp"

TEST(global_metadata_resolution) {
    using namespace i2c::functions;
    initialize();
    CheckS_GlobalMetadata();

    if (!s_GlobalMetadata || !s_GlobalMetadataHeader || !s_Il2CppMetadataRegistration) {
        LOG_FAIL("Global metadata roots contain a null pointer");
        return;
    }
    if (static_cast<uint32_t>(s_GlobalMetadataHeader->sanity) != 0xFAB11BAF) {
        LOG_FAIL("Resolved global metadata header has invalid sanity value");
        return;
    }
    if (s_Il2CppMetadataRegistration->typesCount <= 0 || !s_Il2CppMetadataRegistration->types ||
        s_Il2CppMetadataRegistration->typeDefinitionsSizesCount <= 0 || !s_Il2CppMetadataRegistration->typeDefinitionsSizes) {
        LOG_FAIL("Resolved metadata registration has no type or type-size table");
        return;
    }
    LOG_OK(
        "Global metadata roots resolved (metadata version {}, {} registered types)",
        s_GlobalMetadataHeader->version,
        s_Il2CppMetadataRegistration->typesCount
    );
}

TEST(global_metadata_type_definition_lookup) {
    using namespace i2c::functions;
    initialize();
    CheckS_GlobalMetadata();

    if (!GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex || !s_Il2CppMetadataRegistration ||
        s_Il2CppMetadataRegistration->typeDefinitionsSizesCount <= 0) {
        LOG_FAIL("Type-definition lookup or its metadata registration is unavailable");
        return;
    }

    // Type::GetClass accepts an Il2CppType*, whereas the function being resolved accepts an index.
    // Exercise both ends of the table and the invalid-index sentinel with either Unity 6 resolver.
    for (TypeDefinitionIndex index : {0, s_Il2CppMetadataRegistration->typeDefinitionsSizesCount - 1}) {
        auto* klass = GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex(index);
        if (!klass || type_get_class(const_cast<Il2CppType*>(class_get_type_const(klass))) != klass) {
            LOG_FAIL("Type-definition index {} did not round-trip through Type::GetClass", index);
        } else {
            LOG_OK("Type-definition index {} round-tripped through Type::GetClass", index);
        }
    }
    if (GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex(kTypeDefinitionIndexInvalid) != nullptr) {
        LOG_FAIL("Invalid type-definition index did not return null");
    } else {
        LOG_OK("Invalid type-definition index returned null");
    }
}

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
    auto* corlib = i2c::functions::get_corlib();
    bool found_corlib = false;
    for (size_t i = 0; i < asm_count; ++i) {
        auto* asm_ptr = assemblies[i];
        if (!asm_ptr) {
            LOG_FAIL("Assembly[{}] was null", i);
            continue;
        }
        auto* img = i2c::functions::assembly_get_image(asm_ptr);
        char const* a_name = img ? img->name : nullptr;
        if (a_name) {
            names.emplace_back(a_name);
            LOG_OK("Assembly[{}] -> {}", i, a_name);
            if (i2c::functions::domain_assembly_open(domain, a_name) != asm_ptr) {
                LOG_FAIL("Assembly[{}] did not round-trip by its image name: {}", i, a_name);
            }
            found_corlib |= img == corlib;
        } else {
            LOG_FAIL("Assembly[{}] had null image/name", i);
        }
    }

    // Assembly names vary by game and may include .dll. Check the runtime's actual corlib
    // and lookup results instead of assuming Assembly-CSharp or firstpass exists.
    if (!corlib || !found_corlib) {
        LOG_FAIL("The runtime's corlib image was missing from assembly enumeration");
    } else {
        LOG_OK("Assembly enumeration includes the runtime's corlib image");
    }
    if (i2c::functions::domain_assembly_open(domain, "BSHook.Nonexistent.Assembly.For.Tests.dll")) {
        LOG_FAIL("Assembly lookup unexpectedly resolved a nonexistent assembly");
    } else {
        LOG_OK("Assembly lookup returned null for a nonexistent assembly");
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
