#include <iterator>
#include "flamingo/shared/target-data.hpp"
#include "flamingo/shared/installer.hpp"
#include "flamingo/shared/hook-installation-result.hpp"
#include "flamingo/shared/hook-data.hpp"

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

    flamingo::HookHandle handle;

    FlamingoHandleHelper(flamingo::HookHandle handle) : handle(handle) {

    }

    operator flamingo::HookHandle() const {
        return handle;
    }

  
    flamingo::HookHandle convert() {
        return handle;
    }

    FlamingoHandleHelper& final(bool isFinal = true) {
        handle.hook_location->metadata.priority.is_final = isFinal;
        return *this;
    }

    FlamingoHandleHelper& after(modloader::ModInfo const& info) {
        after(info.id);
        return *this;
    }

    FlamingoHandleHelper& after(std::string_view modID) {
        handle.hook_location->metadata.priority.afters.emplace_back(flamingo::HookNameMetadata{
            .name = std::string(modID)
        });

        return *this;
    }

    FlamingoHandleHelper& before(modloader::ModInfo const& info) {
        before(info.id);
        return *this;
    }

    FlamingoHandleHelper& before(std::string_view modID) {
        handle.hook_location->metadata.priority.befores.emplace_back(flamingo::HookNameMetadata{
            .name = std::string(modID)
        });
        return *this;
    }

    /// @brief Moves this hook to be the last in priority (lowest priority)
    /// @param n Optional offset from last (0 = last, 1 = second last, etc)
    void last(int n = 0) {

    }

    /// @brief Moves this hook to be the first in priority (highest priority)
    /// @param n Optional offset from first (0 = first, 1 = second first, etc)
    void first(int n = 0) {

    }

    /// @brief Gets the current position of this hook in the priority list (0 = first/highest)
    int position() const {
        return 0;
    }

    void reinstall() {
        auto result = flamingo::Reinstall({handle.hook_location->target});
        if (result.has_value()) {
            // Reinstall successful
        }
    }

    // TODO: Allow reinstalling, this currently invalidates the handle though
    void uninstall() {
        auto result = flamingo::Uninstall(handle);
        if (result.has_value()) {
            // Uninstall successful
        }
        // TODO: Error handle
    }
};
}  // namespace bs_hook