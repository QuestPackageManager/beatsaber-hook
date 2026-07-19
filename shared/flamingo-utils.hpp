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

    FlamingoHandleBuilder& final(bool isFinal = true) {
        hookInfo.metadata.priority.is_final = isFinal;
        return *this;
    }

    FlamingoHandleBuilder& after(modloader::ModInfo const& info, std::string_view name = {}) {
        after(info.id, name);
        return *this;
    }

    FlamingoHandleBuilder& after(std::string_view modID, std::string_view name = {}) {
        hookInfo.metadata.priority.afters.emplace_back(flamingo::HookNameMetadata{
            .name = std::string(name),
            .namespaze = std::string(modID),
        });

        return *this;
    }

    FlamingoHandleBuilder& before(modloader::ModInfo const& info, std::string_view name = {}) {
        before(info.id, name);
        return *this;
    }

    FlamingoHandleBuilder& before(std::string_view modID, std::string_view name = {}) {
        hookInfo.metadata.priority.befores.emplace_back(flamingo::HookNameMetadata{ .name = std::string(name), .namespaze = std::string(modID) });
        return *this;
    }

    [[nodiscard]]
    std::expected<FlamingoHandle, flamingo::installation::Error> installOrError() noexcept;

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

    operator flamingo::HookHandle() const {
        return handle;
    }

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
