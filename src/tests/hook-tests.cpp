// #define TEST_HOOK
#ifdef TEST_HOOK
#pragma clang diagnostic push
// these warnings are not relevant here because we are causing them "on purpose" so we disable the warnings here
#pragma clang diagnostic ignored "-Wunused-value"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "../../shared/utils/hooking.hpp"
#include "../../shared/utils/base-wrapper-type.hpp"
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
    ::Hooking ::InstallHook<Hook_test>(il2cpp_utils ::Logger).after("test-mod");

    modloader::ModInfo chroma = { "chroma", "1.0.0", 0 };
    auto& hook = INSTALL_HOOK(il2cpp_utils::Logger, test2_hook).after(chroma).before("test");

    if (false) {
        hook.uninstall();
    }
}

#pragma clang diagnostic pop
#endif
