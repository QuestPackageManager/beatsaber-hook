#include "config.hpp"
#include "tests.hpp"
#include "scotland2/shared/modloader.h"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};

constexpr auto logger = Paper::ConstLoggerContext(MOD_ID);

extern "C" void setup(CModInfo* info) noexcept {
    *info = modInfo.to_c();

    Paper::Logger::RegisterFileContextId(MOD_ID);

    logger.info("Completed setup!");
}

std::vector<std::function<void()>> tests = {};

extern "C" void late_load() {
    logger.info("Beginning tests...");
    for (auto const& test : tests) {
        try {
            test();
        } catch (std::exception const& e) {
            logger.error("Exception in test: {}", e.what());
        } catch (...) {
            logger.error("Unknown exception in test");
        }
    }
    logger.info("Finished tests!");
}
