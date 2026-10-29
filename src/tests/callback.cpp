#include "callback.hpp"

#include "tests.hpp"

#include <algorithm>
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

// Subscriber identity: distinct lambdas and one member function on several instances
// must all be kept, and removal must only affect the matching subscription.
struct IdentityTarget {
    int id;
    std::vector<int>* log;
    void record(int value) { log->push_back(id * 100 + value); }
};

template <template <typename...> typename C>
static void subscribe_lambda(basic_event_callback<C, int>& cb, std::vector<int>& log, int id) {
    // Each call's lambda is a new local at the same stack depth.
    cb += [&log, id](int value) { log.push_back(id * 100 + value); };
}

template <template <typename...> typename C>
static bool check_callback_identity(char const* container) {
    std::vector<int> log;
    auto sorted_log = [&log]() {
        auto copy = log;
        std::sort(copy.begin(), copy.end());
        return copy;
    };

    // Distinct rvalue lambdas, including ones created at the same stack address.
    {
        basic_event_callback<C, int> cb;
        cb += [&log](int value) { log.push_back(100 + value); };
        cb += [&log](int value) { log.push_back(200 + value); };
        for (int id = 3; id <= 5; id++) {
            subscribe_lambda(cb, log, id);
        }
        cb.invoke(1);
        if (cb.size() != 5 || sorted_log() != std::vector<int>{101, 201, 301, 401, 501}) {
            LOG_FAIL("{}: distinct lambdas were merged: size={}, calls={}", container, cb.size(), log.size());
            return false;
        }
    }

    // One member function on two instances; -= removes only the named instance.
    {
        log.clear();
        basic_event_callback<C, int> cb;
        IdentityTarget a{1, &log};
        IdentityTarget b{2, &log};
        cb.add(&IdentityTarget::record, &a);
        cb += {&IdentityTarget::record, &b};
        cb.add(&IdentityTarget::record, &a);  // duplicate subscription stays single
        cb.invoke(7);
        if (cb.size() != 2 || sorted_log() != std::vector<int>{107, 207}) {
            LOG_FAIL("{}: member function on two instances: size={}, calls={}", container, cb.size(), log.size());
            return false;
        }
        log.clear();
        cb -= {&IdentityTarget::record, &a};
        cb.invoke(8);
        if (cb.size() != 1 || log != std::vector<int>{208}) {
            LOG_FAIL("{}: removing one instance's member callback affected the other: size={}", container, cb.size());
            return false;
        }
        cb.add(&IdentityTarget::record, &a);
        cb.remove(&IdentityTarget::record);  // removes the method for every instance
        if (cb.size() != 0) {
            LOG_FAIL("{}: remove(member) left {} callbacks", container, cb.size());
            return false;
        }
    }

    // A lambda is removed through the callback it was added as, not by a different lambda.
    {
        log.clear();
        basic_event_callback<C, int> cb;
        typename basic_event_callback<C, int>::callback_t kept = [&log](int value) { log.push_back(100 + value); };
        cb += kept;
        cb += [&log](int value) { log.push_back(200 + value); };
        cb -= [&log](int value) { log.push_back(100 + value); };
        if (cb.size() != 2) {
            LOG_FAIL("{}: removing a never-added lambda changed size to {}", container, cb.size());
            return false;
        }
        cb -= kept;
        cb.invoke(9);
        if (cb.size() != 1 || log != std::vector<int>{209}) {
            LOG_FAIL("{}: removing a retained lambda failed: size={}", container, cb.size());
            return false;
        }
    }
    return true;
}

TEST(event_callback_subscriber_identity) {
    if (check_callback_identity<std::set>("event_callback") && check_callback_identity<std::unordered_set>("unordered_event_callback")) {
        LOG_OK("event_callback keeps distinct lambdas and per-instance member callbacks, and removes only the matching one");
    }
}
