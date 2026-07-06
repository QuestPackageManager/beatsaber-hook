#include "callback.hpp"

#include "tests.hpp"

#include <string>
#include <vector>

// Helper variables for testing
static int free_func_call_count = 0;
static int member_func_call_count = 0;
static int lambda_call_count = 0;
static std::vector<std::string> call_log;

// Free function callback
void test_free_func(int value) {
    free_func_call_count++;
    call_log.push_back("free:" + std::to_string(value));
}

// Struct with member function callback
struct TestClass {
    void member_func(int value) {
        member_func_call_count++;
        call_log.push_back("member:" + std::to_string(value));
    }
};

// Reset helper variables
void reset_test_state() {
    free_func_call_count = 0;
    member_func_call_count = 0;
    lambda_call_count = 0;
    call_log.clear();
}

// Test basic free function callback
TEST(event_callback_free_func) {
    LOG_OK("Testing event_callback with free function");

    event_callback<int> cb;
    reset_test_state();

    // Add free function
    cb.add(test_free_func);

    // Invoke
    cb.invoke(42);

    // Check results
    if (free_func_call_count == 1 && call_log.size() == 1 && call_log[0] == "free:42") {
        LOG_OK("Free function callback worked correctly");
    } else {
        LOG_FAIL("Free function callback failed: count={}, log_size={}", free_func_call_count, call_log.size());
    }
}

// Test member function callback
TEST(event_callback_member_func) {
    LOG_OK("Testing event_callback with member function");

    event_callback<int> cb;
    TestClass obj;
    reset_test_state();

    // Add member function
    cb.add(&TestClass::member_func, &obj);

    // Invoke
    cb.invoke(123);

    // Check results
    if (member_func_call_count == 1 && call_log.size() == 1 && call_log[0] == "member:123") {
        LOG_OK("Member function callback worked correctly");
    } else {
        LOG_FAIL("Member function callback failed: count=%d, log_size=%d", member_func_call_count, call_log.size());
    }
}

// Test += member function callback
TEST(event_pluseq_callback_member_func) {
    LOG_OK("Testing event_callback with member function");

    event_callback<int> cb;
    TestClass obj;
    reset_test_state();

    // Add member function
    cb += {&TestClass::member_func, &obj};

    // Invoke
    cb.invoke(321);

    // Check results
    if (member_func_call_count == 1 && call_log.size() == 1 && call_log[0] == "member:321") {
        LOG_OK("Member function += worked correctly");
    } else {
        LOG_FAIL("Member function += failed: count=%d, log_size=%d", member_func_call_count, call_log.size());
    }
}

// Test std::function callback
TEST(event_callback_std_function) {
    LOG_OK("Testing event_callback with std::function");

    event_callback<int> cb;
    reset_test_state();

    // Add lambda as std::function
    std::function<void(int)> lambda = [](int value) {
        lambda_call_count++;
        call_log.push_back("lambda:" + std::to_string(value));
    };
    cb.add(lambda);

    // Invoke
    cb.invoke(99);

    // Check results
    if (lambda_call_count == 1 && call_log.size() == 1 && call_log[0] == "lambda:99") {
        LOG_OK("std::function callback worked correctly");
    } else {
        LOG_FAIL("std::function callback failed: count={}, log_size={}", lambda_call_count, call_log.size());
    }
}

// Test multiple callbacks
TEST(event_callback_multiple) {
    LOG_OK("Testing event_callback with multiple callbacks");

    event_callback<int> cb;
    TestClass obj;
    reset_test_state();

    // Add multiple callbacks
    cb.add(test_free_func);
    cb.add(&TestClass::member_func, &obj);
    cb.add([](int value) {
        lambda_call_count++;
        call_log.push_back("lambda:" + std::to_string(value));
    });

    // Invoke
    cb.invoke(77);

    // Check results
    if (free_func_call_count == 1 && member_func_call_count == 1 && lambda_call_count == 1 && call_log.size() == 3 && call_log[0] == "free:77" &&
        call_log[1] == "member:77" && call_log[2] == "lambda:77") {
        LOG_OK("Multiple callbacks worked correctly");
    } else {
        LOG_FAIL(
            "Multiple callbacks failed: free={}, member={}, lambda={}, log_size={}",
            free_func_call_count,
            member_func_call_count,
            lambda_call_count,
            call_log.size()
        );
    }
}

// Test += and -= operators
TEST(event_callback_operators) {
    LOG_OK("Testing event_callback += and -= operators");

    event_callback<int> cb;
    reset_test_state();

    // Add with +=
    cb += test_free_func;

    // Invoke
    cb.invoke(1);
    if (free_func_call_count != 1) {
        LOG_FAIL("+= operator failed to add callback");
        return;
    }

    // Remove with -=
    cb -= test_free_func;

    // Reset and invoke again
    free_func_call_count = 0;
    cb.invoke(2);
    if (free_func_call_count == 0) {
        LOG_OK("+= and -= operators worked correctly");
    } else {
        LOG_FAIL("-= operator failed to remove callback");
    }
}

// Test size and clear
TEST(event_callback_size_clear) {
    LOG_OK("Testing event_callback size and clear");

    event_callback<int> cb;

    // Initially empty
    if (cb.size() != 0) {
        LOG_FAIL("Initial size should be 0, got {}", cb.size());
        return;
    }

    // Add callbacks
    cb.add(test_free_func);
    cb.add([](int) {});
    if (cb.size() != 2) {
        LOG_FAIL("Size after adding should be 2, got {}", cb.size());
        return;
    }

    // Clear
    cb.clear();
    if (cb.size() == 0) {
        LOG_OK("size and clear worked correctly");
    } else {
        LOG_FAIL("clear failed, size is %d", cb.size());
    }
}

// Test remove functionality
TEST(event_callback_remove) {
    LOG_OK("Testing event_callback remove");

    event_callback<int> cb;
    TestClass obj;
    reset_test_state();

    // Add callbacks
    cb.add(test_free_func);
    cb.add(&TestClass::member_func, &obj);
    cb.add([](int) { lambda_call_count++; });

    // Invoke all
    cb.invoke(10);
    if (free_func_call_count != 1 || member_func_call_count != 1 || lambda_call_count != 1) {
        LOG_FAIL("Initial invoke failed");
        return;
    }

    // Remove free function
    cb.remove(test_free_func);
    reset_test_state();
    cb.invoke(20);
    if (free_func_call_count == 0 && member_func_call_count == 1 && lambda_call_count == 1) {
        LOG_OK("Remove free function worked");
    } else {
        LOG_FAIL("Remove free function failed: free={}, member={}, lambda={}", free_func_call_count, member_func_call_count, lambda_call_count);
    }

    // Remove member function
    cb.remove(&TestClass::member_func);
    reset_test_state();
    cb.invoke(30);
    if (member_func_call_count == 0 && lambda_call_count == 1) {
        LOG_OK("Remove member function worked");
    } else {
        LOG_FAIL("Remove member function failed: member={}, lambda={}", member_func_call_count, lambda_call_count);
    }
}

// Test with different argument types
TEST(event_callback_different_args) {
    LOG_OK("Testing event_callback with different argument types");

    event_callback<std::string, int> cb;
    bool called = false;

    cb.add([&called](std::string str, int num) {
        called = true;
        if (str == "test" && num == 42) {
            // Success
        } else {
            called = false;
        }
    });

    cb.invoke("test", 42);

    if (called) {
        LOG_OK("Different argument types worked correctly");
    } else {
        LOG_FAIL("Different argument types failed");
    }
}

// Test unordered_event_callback
TEST(unordered_event_callback_test) {
    LOG_OK("Testing unordered_event_callback");

    unordered_event_callback<int> cb;
    reset_test_state();

    cb.add(test_free_func);
    cb.add([](int) { lambda_call_count++; });

    cb.invoke(55);

    if (free_func_call_count == 1 && lambda_call_count == 1) {
        LOG_OK("unordered_event_callback worked correctly");
    } else {
        LOG_FAIL("unordered_event_callback failed: free={}, lambda={}", free_func_call_count, lambda_call_count);
    }
}
