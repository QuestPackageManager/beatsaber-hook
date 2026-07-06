#include "threading.hpp"

#include "tests.hpp"

TEST(basic) {
    LOG_OK("Starting il2cpp_thread basic test");

    bool thread_ran = false;

    il2cpp_thread t([&]() { thread_ran = true; });
    t.join();

    if (thread_ran) {
        LOG_OK("il2cpp_thread executed successfully");
    } else {
        LOG_FAIL("il2cpp_thread did not execute");
    }
}

TEST(with_args) {
    LOG_OK("Starting il2cpp_thread with args test");

    il2cpp_thread t([](int a, int b) { LOG_OK("il2cpp_thread with args got: {}, {}", a, b); }, 5, 10);
    t.join();

    LOG_OK("il2cpp_thread with args completed");
}

TEST(async_basic) {
    LOG_OK("Starting il2cpp_async basic test");

    auto future = il2cpp_async([]() { return 42; });

    auto result = future.get();
    if (result == 42) {
        LOG_OK("il2cpp_async returned correct value: {}", result);
    } else {
        LOG_FAIL("il2cpp_async returned incorrect value: {}", result);
    }
}

TEST(async_with_args) {
    LOG_OK("Starting il2cpp_async with args test");

    auto future = il2cpp_async([](int a, int b) { return a * b; }, 6, 7);

    auto result = future.get();
    if (result == 42) {
        LOG_OK("il2cpp_async with args returned correct value: {}", result);
    } else {
        LOG_FAIL("il2cpp_async with args returned incorrect value: {}", result);
    }
}

// Test il2cpp_async with exception handling
TEST(async_exception) {
    LOG_OK("Starting il2cpp_async exception test");

    auto future = il2cpp_async([]() {
        throw std::runtime_error("Test exception");
        return 0;
    });

    try {
        auto result = future.get();
        LOG_FAIL("il2cpp_async should have thrown exception, but returned: {}", result);
    } catch (std::runtime_error const& e) {
        if (std::string(e.what()) == "Test exception") {
            LOG_OK("il2cpp_async correctly propagated exception: {}", e.what());
        } else {
            LOG_FAIL("il2cpp_async threw wrong exception: {}", e.what());
        }
    } catch (...) {
        LOG_FAIL("il2cpp_async threw unexpected exception type");
    }
}

TEST(thread_id) {
    LOG_OK("Starting il2cpp_thread thread id test");

    il2cpp_thread t([]() { LOG_OK("Thread ID in il2cpp_thread: {}", i2c::threading::current_thread_id()); });
    LOG_OK("Current thread ID in main thread: {}", i2c::threading::current_thread_id());
    t.join();

    LOG_OK("il2cpp_thread thread id test completed");
}

TEST(jni_env) {
    LOG_OK("Starting il2cpp_thread JNI env test");

    il2cpp_thread t([]() {
        if (i2c::threading::env) {
            LOG_OK("JNI env is set in il2cpp_thread: {}", fmt::ptr(i2c::threading::env));
        } else {
            LOG_FAIL("JNI env is null in il2cpp_thread");
        }
    });
    t.join();

    LOG_OK("il2cpp_thread JNI env test completed");
}

// Test thread id in il2cpp_async
TEST(async_thread_id) {
    LOG_OK("Starting il2cpp_async thread id test");

    auto future = il2cpp_async([]() {
        int tid = i2c::threading::current_thread_id();
        return tid;
    });
    LOG_OK("Thread ID in il2cpp_async: {}", future.get());
}
