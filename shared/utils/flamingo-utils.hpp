#pragma once 

#include <iterator>
#include <optional>
#include <string_view>
#include <variant>
#include "flamingo/shared/hook-data.hpp"
#include "flamingo/shared/hook-installation-result.hpp"
#include "flamingo/shared/installer.hpp"
#include "flamingo/shared/target-data.hpp"

#include "./result.hpp"

namespace bs_hook {
struct FlamingoHandle;

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
    bs_hook::Result<FlamingoHandle, flamingo::installation::Error> installOrError() noexcept;

    [[nodiscard]]
    FlamingoHandle install();
};

struct FlamingoHandle {
    Paper::LoggerContext logger;
    flamingo::HookInfo info;
    flamingo::HookHandle handle;

    FlamingoHandle(Paper::LoggerContext logger, flamingo::HookHandle h, flamingo::HookInfo i) : logger(logger), info(i), handle(h) {}

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
    [[nodiscard]]
    bs_hook::Result<FlamingoHandleBuilder, std::monostate> uninstall() {
        using Result = bs_hook::Result<FlamingoHandleBuilder, std::monostate>;
        auto result = flamingo::Uninstall(handle);
        if (result.has_value()) {
            return Result::Ok(FlamingoHandleBuilder(logger, info));
        }
        return Result::Err(std::monostate{});
    }

    [[nodiscard]]
    auto reinstall() {
        auto result = flamingo::Reinstall({ handle.hook_location->target });
        return result;
    }
};

inline bs_hook::Result<FlamingoHandle, flamingo::installation::Error> FlamingoHandleBuilder::installOrError() noexcept {
    using Result = bs_hook::Result<FlamingoHandle, flamingo::installation::Error>;

    logger.info("Installing hook: {} to offset: {}", hookInfo.metadata.name_info, fmt::ptr(hookInfo.target));
    auto install_result = flamingo::Install(std::move(hookInfo));
    if (install_result.has_value()) {
        return Result::Ok(FlamingoHandle(logger, install_result.value().returned_handle, std::move(hookInfo)));
    } else {
        return Result::Err(install_result.error());
    }
}

inline FlamingoHandle FlamingoHandleBuilder::install() {
    auto result = installOrError();
    if (!result.has_result()) {
        logger.critical("Failed to install hook: {} with flamingo: {}", hookInfo.metadata.name_info, result.get_exception());
        SAFE_ABORT();
    }

    return result.move_result();
}
}  // namespace bs_hook