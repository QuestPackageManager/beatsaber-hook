#include <iterator>
#include <optional>
#include <string_view>
#include "flamingo/shared/hook-data.hpp"
#include "flamingo/shared/hook-installation-result.hpp"
#include "flamingo/shared/installer.hpp"
#include "flamingo/shared/target-data.hpp"

namespace bs_hook {
struct FlamingoHandleHelper {
    FlamingoHandleHelper() = default;
    ~FlamingoHandleHelper() = default;

    // disable copy
    FlamingoHandleHelper(FlamingoHandleHelper const&) = delete;
    FlamingoHandleHelper& operator=(FlamingoHandleHelper const&) = delete;

    // disable move
    FlamingoHandleHelper(FlamingoHandleHelper&&) = delete;
    FlamingoHandleHelper& operator=(FlamingoHandleHelper&&) = delete;

    std::optional<flamingo::HookHandle> handle;
    FlamingoHandleHelper(flamingo::HookHandle handle) : handle(handle) {}

    operator std::optional<flamingo::HookHandle>() const {
        return handle;
    }

    std::optional<flamingo::HookHandle> convert() {
        return handle;
    }

    FlamingoHandleHelper& final(bool isFinal = true) {
        if (!handle.has_value()) {
            return *this;
        }
        handle->hook_location->metadata.priority.is_final = isFinal;
        return *this;
    }

    FlamingoHandleHelper& after(modloader::ModInfo const& info, std::string_view name = {}) {
        after(info.id, name);
        return *this;
    }

    FlamingoHandleHelper& after(std::string_view modID, std::string_view name = {}) {
        if (!handle.has_value()) {
            return *this;
        }
        handle->hook_location->metadata.priority.afters.emplace_back(flamingo::HookNameMetadata{
            .name = std::string(name),
            .namespaze = std::string(modID),
        });

        return *this;
    }

    FlamingoHandleHelper& before(modloader::ModInfo const& info, std::string_view name = {}) {
        before(info.id, name);
        return *this;
    }

    FlamingoHandleHelper& before(std::string_view modID, std::string_view name = {}) {
        if (!handle.has_value()) {
            return *this;
        }
        handle->hook_location->metadata.priority.befores.emplace_back(flamingo::HookNameMetadata{ .name = std::string(name), .namespaze = std::string(modID) });
        return *this;
    }


    FlamingoHandleHelper& reinstall() {
        if (!handle.has_value()) {
            return *this;
        }
        auto result = flamingo::Reinstall({ handle->hook_location->target });
        if (result.has_value()) {
            // Reinstall successful
        }

        return *this;
    }

    // TODO: Allow reinstalling, this currently invalidates the handle though
    /// @brief Uninstalls the hook associated with this handle.
    /// After uninstalling, the handle is no longer valid.
    void uninstall() {
        if (!handle.has_value()) {
            return;
        }
        auto result = flamingo::Uninstall(*handle);
        if (result.has_value()) {
            // Uninstall successful
        }
        handle = std::nullopt;
        // TODO: Error handle
    }
};
}  // namespace bs_hook