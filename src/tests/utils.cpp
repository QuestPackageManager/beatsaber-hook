#include "utils.hpp"

#include "tests.hpp"

TEST(directory_exists) {
    if (direxists("/")) {
        LOG_OK("direxists accepts an existing directory");
    } else {
        LOG_FAIL("direxists rejected an existing directory");
    }

    if (!direxists("/dev/null")) {
        LOG_OK("direxists rejects a non-directory");
    } else {
        LOG_FAIL("direxists accepted a non-directory");
    }

    if (!direxists("/proc/self/beatsaber-hook-missing-directory-test")) {
        LOG_OK("direxists rejects a missing path");
    } else {
        LOG_FAIL("direxists accepted a missing path");
    }

    if (direxists("/proc/self/cwd")) {
        LOG_OK("direxists follows a directory symlink");
    } else {
        LOG_FAIL("direxists rejected a directory symlink");
    }

    if (!direxists("/proc/self/exe")) {
        LOG_OK("direxists rejects a regular-file symlink");
    } else {
        LOG_FAIL("direxists accepted a regular-file symlink");
    }
}
