#pragma once

#include <expected>
#include <iterator>
#include <optional>
#include <string_view>
#include <variant>

#include "config.hpp"
#include "flamingo/shared/hook-data.hpp"
#include "flamingo/shared/hook-installation-result.hpp"
#include "flamingo/shared/installer.hpp"
#include "flamingo/shared/target-data.hpp"
#include "scotland2/shared/modloader.h"

#include "utils.hpp"

namespace bs_hook {
struct FlamingoHandle;

/// @brief Fluent builder for constructing and (re)installing a Flamingo hook with a given priority.
struct FlamingoHandleBuilder {
    Paper::LoggerContext logger;
    flamingo::HookInfo hookInfo;

    FlamingoHandleBuilder(Paper::LoggerContext const& log, flamingo::HookInfo info) : logger(log), hookInfo(std::move(info)) {
        hookInfo.metadata.priority.is_final = false;
    }
    FlamingoHandleBuilder(FlamingoHandleBuilder const&) = default;
    FlamingoHandleBuilder(FlamingoHandleBuilder&&) = default;
    ~FlamingoHandleBuilder() = default;

    FlamingoHandleBuilder& operator=(FlamingoHandleBuilder const&) = default;
    FlamingoHandleBuilder& operator=(FlamingoHandleBuilder&&) = default;

    /// @brief Marks this hook as final, meaning no other hooks may be installed after it.
    FlamingoHandleBuilder& final(bool isFinal = true) {
        hookInfo.metadata.priority.is_final = isFinal;
        return *this;
    }

    /// @brief Requires this hook to be installed after the given mod's hook.
    /// @param info The mod whose hook this one must be installed after.
    /// @param name Restricts the match to a specific hook name within that mod; unset matches any hook in the mod's namespace.
    FlamingoHandleBuilder& after(modloader::ModInfo const& info, std::optional<std::string> name = {}) {
        after(info.id, std::move(name));
        return *this;
    }

    /// @brief Requires this hook to be installed after the hook identified by namespace/name.
    /// @param namespaze The namespace to match; unset matches any namespace.
    /// @param name The hook name to match; unset matches any name.
    FlamingoHandleBuilder& after(std::optional<std::string> namespaze, std::optional<std::string> name = {}) {
        flamingo::HookNameFilter filter;
        filter.namespaze = std::move(namespaze);
        filter.name = std::move(name);
        hookInfo.metadata.priority.afters.emplace_back(std::move(filter));

        return *this;
    }

    /// @brief Requires this hook to be installed before the given mod's hook.
    /// @param info The mod whose hook this one must be installed before.
    /// @param name Restricts the match to a specific hook name within that mod; unset matches any hook in the mod's namespace.
    FlamingoHandleBuilder& before(modloader::ModInfo const& info, std::optional<std::string> name = {}) {
        before(info.id, std::move(name));
        return *this;
    }

    /// @brief Requires this hook to be installed before the hook identified by namespace/name.
    /// @param namespaze The namespace to match; unset matches any namespace.
    /// @param name The hook name to match; unset matches any name.
    FlamingoHandleBuilder& before(std::optional<std::string> namespaze, std::optional<std::string> name = {}) {
        flamingo::HookNameFilter filter;
        filter.namespaze = std::move(namespaze);
        filter.name = std::move(name);
        hookInfo.metadata.priority.befores.emplace_back(std::move(filter));

        return *this;
    }

    /// @brief Installs the hook, returning an error instead of aborting on failure.
    [[nodiscard]]
    std::expected<FlamingoHandle, flamingo::installation::Error> installOrError() noexcept;

    /// @brief Installs the hook, aborting the process on failure.
    [[nodiscard]]
    FlamingoHandle install();
};

/// @brief A handle to an installed Flamingo hook, allowing for uninstalling and reinstalling with a new priority.
struct FlamingoHandle {
    Paper::LoggerContext logger;
    flamingo::HookHandle handle;

    FlamingoHandle(Paper::LoggerContext logger, flamingo::HookHandle h) : logger(logger),  handle(h) {}

    FlamingoHandle(FlamingoHandle const&) = delete;
    FlamingoHandle(FlamingoHandle&&) = default;

    FlamingoHandle& operator=(FlamingoHandle const&) = delete;
    FlamingoHandle& operator=(FlamingoHandle&&) = default;

    ~FlamingoHandle() = default;

    /// @brief Implicit conversion to the underlying Flamingo hook handle.
    operator flamingo::HookHandle() const {
        return handle;
    }

    /// @brief Returns the underlying Flamingo hook handle.
    [[nodiscard]]
    flamingo::HookHandle const& get_handle() const {
        return handle;
    }

    /// @brief Uninstalls the hook associated with this handle.
    [[nodiscard]] std::expected<FlamingoHandleBuilder, std::monostate> uninstall() {
        // grab the hook info and copy it before uninstalling
        auto info = *handle.hook_location;
        auto result = flamingo::Uninstall(handle);
        if (result.has_value()) {
            return FlamingoHandleBuilder(logger, info);
        }
        return std::unexpected(std::monostate{});
    }

    /// @brief Reinstalls the hook at its current target, keeping its existing priority.
    [[nodiscard]]
    auto reinstall() {
        auto result = flamingo::Reinstall({ handle.hook_location->target });
        return result;
    }
};

inline std::expected<FlamingoHandle, flamingo::installation::Error> FlamingoHandleBuilder::installOrError() noexcept {
    logger.info("Installing hook: {} to offset: {}", hookInfo.metadata.name_info, fmt::ptr(hookInfo.target));
    auto install_result = flamingo::Install(std::move(hookInfo));
    if (install_result.has_value()) {
        return FlamingoHandle(logger, install_result.value().returned_handle);
    } else {
        return std::unexpected(install_result.error());
    }
}

inline FlamingoHandle FlamingoHandleBuilder::install() {
    auto result = installOrError();
    if (!result.has_value()) {
        logger.critical("Failed to install hook: {} with flamingo: {}", hookInfo.metadata.name_info, result.error());
        SAFE_ABORT();
    }

    return std::move(result.value());
}
}  // namespace bs_hook
