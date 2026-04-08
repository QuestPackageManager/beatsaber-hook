// #define TEST_HOOK
#include "utils/flamingo-utils.hpp"
#ifdef TEST_HOOK
#pragma clang diagnostic push
// these warnings are not relevant here because we are causing them "on purpose" so we disable the warnings here
#pragma clang diagnostic ignored "-Wunused-value"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "../../shared/utils/base-wrapper-type.hpp"
#include "../../shared/utils/hooking.hpp"
#include "utils/logging.hpp"

MAKE_HOOK(test, 0x0, void, int arg) {
    throw il2cpp_utils::RunMethodException("lol rekt", nullptr);
}

// Method to hook at test2
void* test2(void* one, void*) {
    return one;
}

template <>
struct ::il2cpp_utils::il2cpp_type_check::MetadataGetter<&test2> {
    static const MethodInfo* methodInfo() {
        return nullptr;
    }
};

// Converts FROM void* instances --> the wrapper types when the hook is invoked
MAKE_HOOK_WRAPPER(test2_hook, &test2, bs_hook::Il2CppWrapperType, bs_hook::Il2CppWrapperType one, bs_hook::Il2CppWrapperType two) {
    // Converts from wrapper types --> void* instances to invoke orig
    // Return is a wrapper type
    auto ret = test2_hook(one, two);
    static_assert(std::is_same_v<decltype(ret), bs_hook::Il2CppWrapperType>);
    return ret;
    // Return from overall hook is converted to a void*
}

void install_a_hook() {
    // legacy hook API
    ::Hooking ::InstallHook<Hook_test>(il2cpp_utils ::Logger);

    INSTALL_HOOK(il2cpp_utils::Logger, test2_hook);

    // new Hook API!
    modloader::ModInfo bs_hooks_mod_info = { MOD_ID, "1.0.0", 0 };
    modloader::ModInfo chroma = { "chroma", "1.0.0", 0 };
    auto hook = FLAMINGO_HOOK(il2cpp_utils::Logger, bs_hooks_mod_info, test2_hook).after(chroma).before("test-mod").install();
    auto hook2 = FLAMINGO_HOOK(il2cpp_utils::Logger, "bs_hooks", test2_hook).after(chroma).before("test-mod").install();

    auto doUninstall = false;
    if (doUninstall) {
        // uninstall the hook, which will also remove it from flamingo
        // hook is now invalid after this call
        auto builder = hook.uninstall().get_result();

        builder.after("another-mod").before("test-mod-2").final();
        // reinstall the hook
        hook = builder.install();
    }
}

#pragma region TemporaryHookSetup
std::vector<bs_hook::FlamingoHandleBuilder> to_install_hooks;
std::vector<bs_hook::FlamingoHandle> installed_hooks;


void install_all_hooks() {
    for (auto& builder : to_install_hooks) {
        auto result = builder.installOrError();
        if (result.has_exception()) {
            // Handle installation error (e.g., log it)
            il2cpp_utils::Logger.error("Failed to install hook: {}", builder.hookInfo.metadata.name_info);
        } else {
            // Successfully installed the hook, we can track the handle for potential uninstallation
            installed_hooks.emplace_back(std::move(result.get_result()));
        }
    }
    to_install_hooks.clear();
}

void uninstall_all_hooks() {
    for (auto& hook : installed_hooks) {
        auto result = hook.uninstall();
        if (result.has_exception()) {
            // Handle uninstallation error (e.g., log it)
            il2cpp_utils::Logger.error("Failed to uninstall hook: {}", hook.info.metadata.name_info);
        } else {
            // Successfully uninstalled the hook, we can track the builder for potential reinstallation
            to_install_hooks.emplace_back(std::move(result.get_result()));
        }
    }
    installed_hooks.clear();
}

// Install a Flamingo hook and track it. The trailing chaining methods
// (like `.after(...)` / `.before(...)`) are optional — pass them as
// additional tokens starting with a dot, e.g.
// `TEMP_FLAMINGO_INSTALL(logger, owner, hook_fn, .after(chroma).before("mod"))`
#define TEMP_FLAMINGO_INSTALL(logger, owner, hook_fn, ...) to_install_hooks.emplace_back((FLAMINGO_HOOK(logger, owner, hook_fn) __VA_ARGS__))


// Converts FROM void* instances --> the wrapper types when the hook is invoked
MAKE_HOOK_WRAPPER(test3_install_hooks, &test2, bs_hook::Il2CppWrapperType, bs_hook::Il2CppWrapperType one, bs_hook::Il2CppWrapperType two) {
    // Converts from wrapper types --> void* instances to invoke orig
    // Return is a wrapper type
    auto ret = test3_install_hooks(one, two);
    static_assert(std::is_same_v<decltype(ret), bs_hook::Il2CppWrapperType>);
    
    install_all_hooks();

    return ret;
    // Return from overall hook is converted to a void*
}
// Converts FROM void* instances --> the wrapper types when the hook is invoked
MAKE_HOOK_WRAPPER(test3_uninstall_hooks, &test2, void*, void* one, void* two) {
    // Converts from wrapper types --> void* instances to invoke orig
    // Return is a wrapper type
    auto ret = test3_uninstall_hooks(one, two);
    
    uninstall_all_hooks();

    return ret;
    // Return from overall hook is converted to a void*
}

void build_temporary_hooks() {
    // new Hook API!

    modloader::ModInfo bs_hooks_mod_info = { MOD_ID, "1.0.0", 0 };
    modloader::ModInfo chroma = { "chroma", "1.0.0", 0 };
    TEMP_FLAMINGO_INSTALL(il2cpp_utils::Logger, bs_hooks_mod_info, test2_hook, .after(chroma).before("test-mod"));
    TEMP_FLAMINGO_INSTALL(il2cpp_utils::Logger, "bs_hooks", test2_hook, .after(chroma).before("test-mod"));
    TEMP_FLAMINGO_INSTALL(il2cpp_utils::Logger, "bs_hooks", test2_hook, .final());
    TEMP_FLAMINGO_INSTALL(il2cpp_utils::Logger, "bs_hooks", test2_hook);

    auto installer = FLAMINGO_HOOK(il2cpp_utils::Logger, bs_hooks_mod_info, test3_install_hooks)
        .after(chroma)
        .before("test-mod")
        .install();
    auto uninstaller = FLAMINGO_HOOK(il2cpp_utils::Logger, bs_hooks_mod_info, test3_uninstall_hooks)
        .after(chroma)
        .before("test-mod")
        .install();
}

#pragma endregion

#pragma clang diagnostic pop
#endif
