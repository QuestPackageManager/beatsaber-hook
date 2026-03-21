#pragma once

#include "config.hpp"
#include "paper2_scotland2/shared/backtrace.hpp"

#include <optional>
#include <string_view>
#include <thread>
#include <cxxabi.h>
#include <dlfcn.h>

namespace i2c {
    template <typename T>
    auto&& unwrap_optionals(T&& arg) {
        return arg;
    }

    template <typename T>
    auto&& unwrap_optionals(std::optional<T>&& arg) {
        return *arg;
    }

    template <typename... TArgs>
    BS_HOOK_NO_RETURN inline void safe_abort(Paper::FmtStrSrcLoc<TArgs...> fmt, TArgs&&... args) {
        // Make sure the message appears at least once in the log
        for (int i = 0; i < 2; i++) {
            usleep(100000L);  // 0.1s
            logger.critical(fmt, args...);
        }
        logger.Backtrace(512);
        Paper::Logger::WaitForFlush();
        usleep(100000L);  // 0.1s
        std::terminate();  // Cleans things up and then calls abort
    }

    BS_HOOK_NO_RETURN inline void safe_abort() {
        safe_abort("Aborting");
    }

    // from https://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template#comment77016419_19123821 (https://ideone.com/sqFWir)
    template <typename T>
    std::string type_name() {
        std::string name = typeid(T).name();
#if defined(__clang__) || defined(__GNUG__)
        int status;
        char* demangled_name = abi::__cxa_demangle(name.c_str(), NULL, NULL, &status);
        if (status == 0) {
            name = demangled_name;
            std::free(demangled_name);
        }
#endif
        return name;
    }

    // From https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
    template <typename T>
    size_t hash_combine(T const& val, size_t seed = 0) {
        return seed ^ std::hash<std::decay_t<T>>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    template <typename... TArgs>
    size_t hash_combine(TArgs const&... vals) {
        size_t seed = 0;
        ((seed = hash_combine(vals, seed)), ...);
        return seed;
    }

    /// Helper type to run operator () on something once this variable goes out of scope
    template <typename F>
    requires(std::is_invocable_v<F>)
    struct on_scope_exit {
        inline on_scope_exit(F f) : f(f) {}
        inline ~on_scope_exit() { f(); }
        F f;
    };

    // Restores an existing stringstream to a newly created state.
    void reset_ss(std::stringstream& ss);
    // Prints the given number of "tabs" as spaces to the given output stream.
    void tabs(std::ostream& os, int tabs, int spacesPerTab = 2);
}

// function_ptr_t courtesy of DaNike
template <typename TRet, typename... TArgs>
using function_ptr_t = TRet (*)(TArgs...);

// Like function_ptr_t, but for an instance method
template <typename T, typename TRet, typename... TArgs>
using method_ptr_t = TRet (T::*)(TArgs...);

// Creates all directories for a provided file path
// Ex: /sdcard/Android/data/something/files/libs/
int mkpath(std::string_view file_path);
// Reads all of the text of a file at the given filename. If the file does not exist, returns an empty string.
std::string readfile(std::string_view filename);
// Reads all bytes from the provided file at the given filename. If the file does not exist, returns an empty vector.
std::vector<char> readbytes(std::string_view filename);
// Writes all of the text to a file at the given filename. Returns true on success, false otherwise
bool writefile(std::string_view filename, std::string_view text);
// Deletes a file at the given filename. Returns true on success, false otherwise
bool deletefile(std::string_view filename);
// Returns if a file exists and can be written to / read from
bool fileexists(std::string_view filename);
// Returns if a directory exists and can be written to / read from
bool direxists(std::string_view dirname);

/// @brief Returns a path to the persistent data directory for ID.
/// @param id The id to find a path for.
/// @return The path to the directory.
std::string get_data_dir(std::string_view id);

/// @brief Returns a path to the persistent data directory for the provided const ModInfo&.
/// @param info The const ModInfo& to find a path for.
/// @return The path to the directory.
inline std::string get_data_dir(modloader::ModInfo const& info) {
    return get_data_dir(info.id);
}

/// @brief Returns a path to the persistent configuration file for ID.
/// @param id The id to find a path for.
/// @return The path to the config.
std::string get_config_path(std::string_view id);

/// @brief Returns a path to the persistent configuration file for the provided const ModInfo&.
/// @param info The const ModInfo& to find a path for.
/// @return The path to the config.
inline std::string get_config_path(modloader::ModInfo const& info) {
    return get_config_path(info.id);
}

// Allows concatenation of macros such as __LINE__ to other statements
#define CONCAT_WRAPPED(x, y) x##y
#define CONCAT(x, y) CONCAT_WRAPPED(x, y)

// Thank god for this GCC ({}) extension which "evaluates to the last statement"
#define BS_HOOK_DO_UNLESS(err, logger, ...) ({                     \
    auto&& __temp__ = (__VA_ARGS__);                               \
    if (!__temp__) {                                               \
        MACRO_LOG(logger, error, #__VA_ARGS__ " returned false!"); \
        err;                                                       \
    }                                                              \
    ::i2c::unwrap_optionals(__temp__); })

#define THROW_UNLESS(logger, ...) \
    BS_HOOK_DO_UNLESS(throw std::runtime_error(#__VA_ARGS__ " returned false!"), logger, __VA_ARGS__)

// Logs error and RETURNS argument 1 IFF argument 2 boolean evaluates as false; else EVALUATES to argument 2
#define RET_UNLESS(retval, logger, ...) \
    BS_HOOK_DO_UNLESS(return retval, logger, __VA_ARGS__)

#define RET_DEF_UNLESS(logger, ...) \
    BS_HOOK_DO_UNLESS(return {}, logger, __VA_ARGS__)

#define RET_V_UNLESS(logger, ...) \
    BS_HOOK_DO_UNLESS(return, logger, __VA_ARGS__)

#define CRASH_UNLESS(...) \
    BS_HOOK_DO_UNLESS(::i2c::safe_abort(), ::i2c::logger, __VA_ARGS__)

#define SAFE_ABORT(...) \
    ::i2c::safe_abort(__VA_ARGS__)
