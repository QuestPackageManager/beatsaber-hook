#pragma once

#include "flamingo-utils.hpp"
#include "hooking.hpp"
#include "scotland2/shared/modloader.h"

namespace i2c {
    namespace detail {
        // Unlike detail::hook_struct, a flamingo_hook_struct does not carry an install_priority of its own: priority
        // is set per-install via the FlamingoHandleBuilder's fluent methods instead, since the same hook may be
        // installed and uninstalled multiple times independently, each with its own priority. It still has an
        // install_handle slot available, so a caller may optionally store the resulting handle on T itself.
        template <typename T>
        concept flamingo_hook_struct =
            requires {
                // Must have a name
                { T::name() } -> std::same_as<char const*>;
                // Must have a trampoline that returns the func_t
                { T::trampoline() } -> std::same_as<typename T::func_t*>;
                // Must have a hook that returns the func_t
                { T::hook() } -> std::same_as<typename T::func_t>;
                // Must have an installation handle
                { T::install_handle } -> std::same_as<flamingo::HookHandle&>;
            } &&
            (
                // Must have an address
                requires {
                    { T::addr() } -> std::same_as<void*>;
                } ||
                requires {
                    { T::addr() } -> std::same_as<MethodInfo const*>;
                }
            );
    }

    // Builds a Flamingo hook builder for the provided hook type, resolving its address (or using the one provided),
    // for installation to a specific mod namespace with an optional priority (before/after/final) attached via the
    // builder's fluent methods. The resulting handle is not tracked on T, so the same hook may be installed and
    // uninstalled multiple times independently, each with its own priority.
    template <detail::flamingo_hook_struct T>
    bs_hook::FlamingoHandleBuilder make_flamingo_hook(Paper::LoggerContext const& logger, std::string_view namespaze, void* addr = nullptr) {
        if (!addr) {
            auto info_or_addr = T::addr();
            if constexpr (std::is_same_v<decltype(info_or_addr), void*>) {
                addr = info_or_addr;
            } else if (info_or_addr) {
                addr = reinterpret_cast<void*>(info_or_addr->methodPointer);
            } else {
                MACRO_LOG(logger, critical, "Attempting to make flamingo hook: {}, but method could not be found!", T::name());
                SAFE_ABORT("Failure making flamingo hook: {}", T::name());
            }
        }
        if (!addr) {
            MACRO_LOG(logger, critical, "Attempting to make flamingo hook: {} to invalid destination!", T::name());
            SAFE_ABORT("Failure making flamingo hook: {}", T::name());
        }
        flamingo::HookInfo hookInfo(
            reinterpret_cast<void*>(T::hook()), addr, reinterpret_cast<void**>(T::trampoline()),
            flamingo::HookNameMetadata{ .name = T::name(), .namespaze = std::string(namespaze) }
        );
        return bs_hook::FlamingoHandleBuilder(logger, std::move(hookInfo));
    }

    // modloader::ModInfo overload of make_flamingo_hook, using the mod's ID as the hook namespace.
    template <detail::flamingo_hook_struct T, typename... TArgs>
    bs_hook::FlamingoHandleBuilder make_flamingo_hook(Paper::LoggerContext const& logger, modloader::ModInfo const& modInfo, TArgs&&... args) {
        return make_flamingo_hook<T>(logger, std::string_view(modInfo.id), std::forward<TArgs>(args)...);
    }
}

#define __INTERNAL_FLAMINGO_HOOK_STRUCT(name_, addr_, ret_type, ...) \
    constexpr static const char* name() { return #name_; }          \
    static auto addr() { return addr_; }                            \
    static func_t hook() { return hook_m_##name_; }                 \
    static func_t* trampoline() { return &name_; }                  \
    static inline flamingo::HookHandle install_handle;               \
    static ret_type hook_m_##name_(__VA_ARGS__); /* Hook */         \
    static inline ret_type (*name_)(__VA_ARGS__) = nullptr; /* Orig */

// Defines a hook to a manually found address or il2cpp method, for use with FLAMINGO_HOOK/FLAMINGO_HOOK_DIRECT.
// addr_info must be in parentheses, and can either be an expression that produces a pointer,
// or a find_class_info, method name, and boolean flag if an instance method.
// If given an il2cpp method, it will search for one that matches the given return type and parameters.
// Unlike MAKE_HOOK, this does not carry an install_priority of its own (see FLAMINGO_HOOK).
#define MAKE_FLAMINGO_HOOK(name_, addr_info, ret_type, ...)                                                        \
    struct BS_HOOK_HIDDEN hook_##name_ {                                                                          \
        using func_t = ret_type (*)(__VA_ARGS__);                                                                 \
        __INTERNAL_FLAMINGO_HOOK_STRUCT(name_, ::i2c::detail::resolve_addr<func_t>{} addr_info, ret_type, __VA_ARGS__) \
    };                                                                                                             \
    ret_type hook_##name_::hook_m_##name_(__VA_ARGS__)

// Defines a hook to a method with metadata provided through i2c::metadata_getter, for use with
// FLAMINGO_HOOK/FLAMINGO_HOOK_DIRECT. Will automatically cast overloads, check types, and detect static/instance
// methods, based on the given return type and parameters. Generic methods cannot be hooked with this macro.
// Unlike MAKE_HOOK_MATCH, this does not carry an install_priority of its own (see FLAMINGO_HOOK).
#define MAKE_FLAMINGO_HOOK_MATCH(name_, method, ret_type, ...)                                                       \
    struct BS_HOOK_HIDDEN hook_##name_ {                                                                            \
        static constexpr auto cast_test = []<typename T>() { return requires { static_cast<T>(method); }; };        \
        using func_t = ret_type (*)(__VA_ARGS__);                                                                   \
        using cast_t = ::i2c::detail::method_check<cast_test, func_t>::type;                                       \
        static_assert(cast_test.operator()<cast_t>(), "Hook method signature does not match!");                     \
        static_assert(::i2c::detail::match_hookable<static_cast<cast_t>(method)>, "Method cannot be hooked!");      \
        __INTERNAL_FLAMINGO_HOOK_STRUCT(name_, ::i2c::metadata_getter<static_cast<cast_t>(method)>::method_info(), ret_type, __VA_ARGS__) \
    };                                                                                                               \
    ret_type hook_##name_::hook_m_##name_(__VA_ARGS__)

// Builds a FlamingoHandleBuilder for the provided hook, namespaced under the given mod ID (either a modloader::ModInfo
// or a plain string). Chain .before(...)/.after(...)/.final() and finish with .install() or .installOrError().
// Name must be from either a MAKE_FLAMINGO_HOOK or MAKE_FLAMINGO_HOOK_MATCH macro.
#define FLAMINGO_HOOK(logger, namespaze, name) \
    ::i2c::make_flamingo_hook<hook_##name>(logger, namespaze)

// Same as FLAMINGO_HOOK, but installs to the address provided directly instead of resolving it from the hook.
// Name must be from either a MAKE_FLAMINGO_HOOK or MAKE_FLAMINGO_HOOK_MATCH macro.
#define FLAMINGO_HOOK_DIRECT(logger, namespaze, name, addr) \
    ::i2c::make_flamingo_hook<hook_##name>(logger, namespaze, addr)
