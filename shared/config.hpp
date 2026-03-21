#pragma once

#include "paper2_scotland2/shared/logger.hpp"

namespace i2c {
    static constexpr auto const logger = Paper::ConstLoggerContext("beatsaber-hook");
}

#ifndef SUPPRESS_MACRO_LOGS
#define MACRO_LOG(logger, severity, ...) \
    logger.severity(__VA_ARGS__)
#else
#define MACRO_LOG(logger, severity, ...)
#endif

#ifndef PERSISTENT_DIR
#define PERSISTENT_DIR "/sdcard/ModData/{}/Mods/"
#endif
#ifndef CONFIG_PATH_FORMAT
#define CONFIG_PATH_FORMAT "/sdcard/ModData/{}/Configs/"
#endif

#define BS_HOOK_HIDDEN __attribute__((visibility("hidden")))
#define BS_HOOK_ALWAYS_INLINE __attribute__((alwaysinline))
#define BS_HOOK_NO_RETURN __attribute__((noreturn))
#define BS_HOOK_DLOPEN __attribute__((constructor))
