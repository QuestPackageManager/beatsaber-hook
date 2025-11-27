#include "../../shared/utils/hook-tracker.hpp"
#include "../../shared/utils/capstone-utils.hpp"
#include "../../shared/utils/logging.hpp"
#include "scotland2/shared/modloader.h"

std::unordered_map<const void*, std::list<HookInfo>> HookTracker::hooks;

void HookTracker::AddHook(HookInfo info) noexcept {
    auto itr = hooks.find(info.destination);
    if (itr == hooks.end()) {
        hooks.emplace(info.destination, std::list<HookInfo>()).first->second.emplace_back(info);
    } else {
        itr->second.emplace_back(info);
    }
}

void HookTracker::RemoveHook(HookInfo info) noexcept {
    auto itr = hooks.find(info.destination);
    if (itr != hooks.end()) {
        for (auto& item : itr->second) {
            if (item == info) {
                itr->second.remove(item);
            }
        }
    }
}

void HookTracker::RemoveHooks() noexcept {
    hooks.clear();
}

void HookTracker::RemoveHooks(const void* const location) noexcept {
    auto itr = hooks.find(location);
    if (itr != hooks.end()) {
        hooks.erase(itr);
    }
}

bool HookTracker::IsHooked(const void* const location) noexcept {
    CombineHooks();
    auto itr = hooks.find(location);
    if (itr != hooks.end()) {
        return itr->second.size() > 0;
    }
    return false;
}

const std::list<HookInfo> HookTracker::GetHooks(const void* const location) noexcept {
    CombineHooks();
    auto itr = hooks.find(location);
    if (itr != hooks.end()) {
        return itr->second;
    }
    return std::list<HookInfo>();
}

const std::unordered_map<const void*, std::list<HookInfo>>* HookTracker::GetHooks() noexcept {
    return &hooks;
}

const void* HookTracker::GetOrigInternal(const void* const location) noexcept {
    CombineHooks();
    auto itr = hooks.find(location);
    if (itr != hooks.end() && itr->second.size() > 0) {
        return itr->second.front().orig;
    }
    return location;
}

#include <dirent.h>

void HookTracker::CombineHooks() noexcept {
    auto const& logger = il2cpp_utils::Logger;
    auto libsFolder = modloader::get_modloader_root_load_path()/"libs";
    auto const& tmpPath = modloader::get_files_dir() / "libs";
    DIR* dir = opendir(libsFolder.c_str());
    if (dir == nullptr) {
        logger.warn("Failed to open libs folder! At path: {}", libsFolder.c_str());
        return;
    }
    struct dirent* dp;
    while ((dp = readdir(dir)) != NULL) {
        if (std::string(dp->d_name).starts_with("libbeatsaber-hook")) {
            // Find and combine
            auto path = tmpPath / dp->d_name;
            auto* image = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
            auto* err = dlerror();
            if (image == nullptr || err != nullptr) {
                logger.warn("Failed to dlopen: {}! {}", path.c_str(), err);
                continue;
            }
            // Open the library, look for a function called: __HOOKTRACKER_GET_HOOKS
            auto* getter = dlsym(image, "__HOOKTRACKER_GET_HOOKS");
            if (getter == nullptr) {
                logger.warn("Failed to find symbol: {}", "__HOOKTRACKER_GET_HOOKS");
                continue;
            }
            // Of course, if the function returns something that is of a different HookInfo type, for example, this may cause all sorts of pain.
            auto otherHooks = *reinterpret_cast<const std::unordered_map<const void*, std::list<HookInfo>>* (*)()>(getter)();
            logger.debug("Found other hooks: {} for module: {}", otherHooks.size(), path.c_str());
            for (auto itr : otherHooks) {
                // For each void*, find our match
                auto match = hooks.find(itr.first);
                if (match == hooks.end()) {
                    hooks.insert({ itr.first, itr.second });
                } else {
                    // Add only unique items
                    for (auto item : itr.second) {
                        bool unique = true;
                        for (auto& existing : match->second) {
                            if (existing == item) {
                                unique = false;
                                break;
                            }
                        }
                        if (unique) {
                            match->second.push_back(item);
                        }
                    }
                }
            }
            dlclose(image);
        }
    }
}

extern "C" const void* __HOOKTRACKER_GET_HOOKS() {
    return reinterpret_cast<const void*>(HookTracker::GetHooks());
}

std::optional<uint32_t*> getHookBr(cs_insn* ins) {
    return (ins->id == ARM64_INS_BR && ins->detail->arm64.operands[0].reg == ARM64_REG_X17) ? std::optional<uint32_t*>(reinterpret_cast<uint32_t*>(ins->address)) : std::nullopt;
}

bool HookTracker::InstructionIsHooked(const void* const location) noexcept {
    // TODO: Rewrite this using flamingo knowledge
    auto brAddr = cs::findNth<1, &getHookBr, &cs::insnMatch<>, -1, 20>(reinterpret_cast<const uint32_t*>(location)).value();
    // br should be second or third instruction, following 8 bytes should be dst to jump to.
    return brAddr == reinterpret_cast<const uint32_t*>(location) + 1 || brAddr == reinterpret_cast<const uint32_t*>(location) + 2;
    // // The third instruction should be a BR x17
    // Instruction instr(reinterpret_cast<const int32_t *>(location) + 2);
    // return instr.isIndirectBranch() && instr.numSourceRegisters == 1 && instr.Rs[0] == 17;
}