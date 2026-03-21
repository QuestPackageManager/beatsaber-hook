#pragma once

#include "config.hpp"

extern std::vector<std::function<void()>> tests;

#define TEST(name)                                                                 \
    static void name();                                                            \
    void __attribute__((constructor)) add_test_##name() { tests.push_back(name); } \
    static void name()

#define LOG_OK(fmt, ...) i2c::logger.info("OK: " fmt, ##__VA_ARGS__)
#define LOG_FAIL(fmt, ...) i2c::logger.warn("FAIL: " fmt, ##__VA_ARGS__)
