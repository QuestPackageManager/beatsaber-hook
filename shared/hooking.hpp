#pragma once

#include "find.hpp"
#include "flamingo/shared/hook-data.hpp"
#include "flamingo/shared/hook-metadata.hpp"
#include "flamingo/shared/installer.hpp"

#ifndef __aarch64__
#error Hooking is only supported on ARM64 platforms!
#endif

namespace i2c::hooking {
    template <typename T>
    concept logger = requires(T& l) {
        l.info("");
        l.debug("");
        l.error("");
        l.warn("");
        l.critical("");
    };

    template <typename T>
    concept hook_struct =
        requires {
            // Must have a name
            { T::name() } -> std::same_as<char const*>;
            // Must have a trampoline that returns the func_t
            { T::trampoline() } -> std::same_as<typename T::func_t*>;
            // Must have a hook that returns the func_t
            { T::hook() } -> std::same_as<typename T::func_t>;
            // Must have an installation handle
            { T::install_handle } -> std::same_as<flamingo::HookHandle>;
            // Must have installation priority
            { T::install_priority } -> std::same_as<flamingo::HookPriority>;
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

    template <typename T>
    struct resolve_addr;

    template <typename R>
    struct resolve_addr<R (*)()> {
        template <typename T>
        auto operator()(T* addr) {
            return reinterpret_cast<void*>(addr);
        }
        auto operator()(MethodInfo const* addr) { return addr; }
        auto operator()(find_class_info klass, std::string_view name) { return find_method(klass, {name, 0}); }
    };

    template <typename R, typename T1, typename... TArgs>
    struct resolve_addr<R (*)(T1, TArgs...)> {
        auto operator()(find_class_info klass, std::string_view name, bool instance = std::is_pointer_v<T1> && type_check::valid_type<T1>) {
            auto args = instance ? std::initializer_list{class_of<TArgs>()...} : std::initializer_list{class_of<T1>(), class_of<TArgs>()...};
            return find_method(klass, {name, {}, args});
        }
    };

    // Used to check overloaded match hooks against the parameter list
    template <auto C, class T>
    // Default fallback to static method type
    struct method_check {
        using type = T;
    };
    template <auto C, class R, class T, class... Ts>
    // Uses C (a constexpr verification function) to see if an instance method (and a particular overload) can be used
    requires(C.template operator()<method_ptr_t<R, T, Ts...>>())
    struct method_check<C, R (*)(T*, Ts...)> {
        using type = R (T::*)(Ts...);
    };

    template <auto M>
#ifndef BS_HOOK_MATCH_UNSAFE
    concept match_hookable = metadata_getter<M>::size >= 0x5 * sizeof(uint32_t) && metadata_getter<M>::addrs != 0x0;
#else
    concept match_hookable = true;
#endif

    template <hook_struct T, logger L>
    void install_hook(L& logger = ::i2c::logger, void* addr = nullptr) {
        if (!addr) {
            auto info_or_addr = T::addr();
            if constexpr (std::is_same_v<decltype(info_or_addr), void*>) {
                addr = info_or_addr;
            } else if (info_or_addr) {
                addr = info_or_addr->methodPointer;
            } else {
                MACRO_LOG(logger, critical, "Attempting to install hook: {}, but method could not be found!", T::name());
                SAFE_ABORT("Failure installing hook: {}", T::name());
            }
        }
        if (!addr) {
            MACRO_LOG(logger, critical, "Attempting to install hook: {} to invalid destination!", T::name());
            SAFE_ABORT("Failure installing hook: {}", T::name());
        }
        MACRO_LOG(logger, info, "Installing hook: {} to offset: {}", T::name(), fmt::ptr(addr));
        auto install_result = flamingo::Install(
            flamingo::HookInfo{
                reinterpret_cast<void*>(T::hook()),
                addr,
                reinterpret_cast<void**>(T::trampoline()),
                flamingo::HookNameMetadata{.name = T::name()},
                T::install_priority
            }
        );
        if (install_result.has_value()) {
            MACRO_LOG(logger, info, "Hook: {} installed with flamingo!", T::name());
            T::install_handle = install_result.value().returned_handle;
        } else {
            MACRO_LOG(logger, critical, "Failed to install hook: {} with flamingo: {}", T::name(), install_result.error());
            SAFE_ABORT("Failure installing hook: {}", T::name());
        }
    }

    template <hook_struct T, logger L>
    void install_hook_orig(L& logger = ::i2c::logger, void* addr = nullptr) {
        T::install_priority.is_final = true;
        install_hook<T>(logger, addr);
    }

    template <hook_struct T, logger L>
    void uninstall_hook(L& logger = ::i2c::logger) {
        MACRO_LOG(logger, info, "Uninstalling hook: {}", T::name());
        auto uninstall_result = flamingo::Uninstall(T::install_handle);
        if (uninstall_result.has_value()) {
            MACRO_LOG(logger, info, "Hook: {} uninstalled with flamingo!", T::name());
            T::install_handle = {};
        } else if (uninstall_result.error()) {
            MACRO_LOG(logger, error, "Failed to uninstall hook: {} with flamingo: remapping error", T::name());
        } else {
            MACRO_LOG(logger, error, "Failed to uninstall hook: {} with flamingo: no target found (was the hook installed?)", T::name());
        }
    }
}

#define __INTERNAL_HOOK_STRUCT(name_, addr_, ret_type, ...) \
    constexpr static const char* name() { return #name_; }  \
    static void* addr() { return addr_; }                   \
    static func_t hook() { return hook_##name_; }           \
    static func_t* trampoline() { return &name_; }          \
    static inline flamingo::HookHandle install_handle;      \
    static inline flamingo::HookPriority install_priority;  \
    static retval hook_##name_(__VA_ARGS__); /* Hook */     \
    static inline retval (*name_)(__VA_ARGS__) = nullptr; /* Orig */

// Defines a hook to a manually found address or il2cpp method.
// addr_info must be in parentheses, and can either be an expression that produces a pointer,
// or a find_class_info, method name, and boolean flag if an instance method.
// If given an il2cpp method, it will search for one that matches the given return type and parameters.
#define MAKE_HOOK(name_, addr_info, ret_type, ...)                                                           \
    struct hook_##name_ {                                                                                    \
        using func_t = ret_type (*)(__VA_ARGS__);                                                            \
        __INTERNAL_HOOK_STRUCT(name_, ::i2c::hooking::resolve_addr<func_t> addr_info, ret_type, __VA_ARGS__) \
    };                                                                                                       \
    retval hook_##name_::hook_##name_(__VA_ARGS__)

// Defines a hook to a method with metadata provided through i2c::metadata_getter.
// Will automatically cast overloads, check types, and detect static/instance methods, based on the given return type and parameters.
// Generic methods cannot be hooked with this macro.
#define MAKE_HOOK_MATCH(name_, method, ret_type, ...)                                                                            \
    struct hook_##name_ {                                                                                                        \
        static constexpr auto cast_test = []<class T>() { return requires { static_cast<T>(method); }; };                        \
        using func_t = ret_type (*)(__VA_ARGS__);                                                                                \
        using cast_t = ::i2c::hooking::method_check<cast_test, func_t>::type;                                                    \
        static_assert(cast_test.operator()<cast_t>(), "Hook method signature does not match!");                                  \
        static_assert(match_hookable<static_cast<cast_t>(method)>, "Method cannot be hooked!");                                  \
        __INTERNAL_HOOK_STRUCT(name_, ::i2c::metadata_getter<static_cast<cast_t>(method)>::method_info(), ret_type, __VA_ARGS__) \
    };                                                                                                                           \
    retval hook_##name_::hook_##name_(__VA_ARGS__)

// Tells a hook to be installed as the final hook, if other hooks are installed to the same target.
#define HOOK_ORIG(name_)                                \
    BS_HOOK_DLOPEN static void hook_##name_##_orig() {  \
        hook_##name_::install_priority.is_final = true; \
    }

// Tells a hook to be installed before another hook, based on the other hook's name.
#define HOOK_BEFORE(name_, before_name)                                    \
    BS_HOOK_DLOPEN static void CONCAT(hook_##name_##_before, __LINE__)() { \
        hook_##name_::install_priority.befores.emplace_back(before_name);  \
    }

// Tells a hook to be installed after another hook, based on the other hook's name.
#define HOOK_AFTER(name_, after_name)                                     \
    BS_HOOK_DLOPEN static void CONCAT(hook_##name_##_after, __LINE__)() { \
        hook_##name_::install_priority.afters.emplace_back(after_name);   \
    }

// Installs the provided hook using the logger provided, and optionally directly to an address.
// Name must be from either a MAKE_HOOK or MAKE_HOOK_MATCH macro.
#define INSTALL_HOOK(logger, name, ...) \
    ::i2c::hooking::install_hook<hook_##name>(logger __VA_OPT__(,) __VA_ARGS__);

// Installs the provided hook using the logger provided, and optionally directly to an address.
// The hook will be forced to be the final hook if multiple are installed to the same target.
// Name must be from either a MAKE_HOOK or MAKE_HOOK_MATCH macro.
#define INSTALL_HOOK_ORIG(logger, name, ...) \
    ::i2c::hooking::install_hook_orig<hook_##name>(logger __VA_OPT__(,) __VA_ARGS__);

// Uninstalls a previously installed hook.
// Name must be from either a MAKE_HOOK or MAKE_HOOK_MATCH macro.
#define UNINSTALL_HOOK(logger, name) \
    ::i2c::hooking::uninstall_hook<hook_##name>(logger);
