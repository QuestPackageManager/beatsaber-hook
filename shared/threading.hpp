#pragma once

#include "types.hpp"

#include <future>

namespace i2c::threading {
    static inline thread_local JNIEnv* env;
    static inline int current_thread_id() {
        return gettid();
    }

    // TODO: Custom reimplementation (Removed in Unity 2023)
    // ref https://github.com/vfsfitvnm/frida-il2cpp-bridge/issues/618#issuecomment-2835645754
#if defined(UNITY_2019) || defined(UNITY_2021)
    /// @brief gets whether the current thread is attached to il2cpp
    /// @return true for attached, false for not attached
    static inline bool is_thread_attached() {
        functions::initialize();
        auto curr_thread = functions::thread_current();
        // if there is no current thread might as well just return false since we didn't get a thread
        if (!curr_thread) {
            return false;
        }

        size_t thread_count = 0;
        auto threads_begin = functions::thread_get_all_attached_threads(&thread_count);
        auto threads_end = threads_begin + thread_count;

        return std::find(threads_begin, threads_end, curr_thread) != threads_end;
    }
#endif

    static inline Il2CppThread* attach_thread() {
        logger.info("Attaching thread {}", current_thread_id());
        functions::initialize();
        // il2cpp attach
        auto domain = functions::domain_get();
        auto thread = functions::thread_attach(domain);
        // jvm attach
        modloader_jvm->AttachCurrentThread(&env, nullptr);
        return thread;
    }

    static inline void detach_thread(Il2CppThread* thread) {
        logger.info("Detaching thread {}", current_thread_id());
        // il2cpp detach
        functions::initialize();
        functions::thread_detach(thread);
        // jvm detach
        modloader_jvm->DetachCurrentThread();
        env = nullptr;
    }

    template <typename F, typename... TArgs>
    requires(std::is_invocable_v<F, TArgs...>)
    static inline std::invoke_result_t<F, TArgs...> attached_invoke(F&& func, TArgs&&... args) {
        auto thread = attach_thread();
        // helper to detach thread on out of scope
        on_scope_exit on_exit([&thread]() { detach_thread(thread); });

        return std::invoke(std::forward<F>(func), std::forward<TArgs>(args)...);
    }

    template <typename F, typename... TArgs>
    requires(std::is_invocable_v<F, TArgs...>)
    static inline std::invoke_result_t<F, TArgs...> il2cpp_async_internal(F&& func, TArgs&&... args) {
#if defined(UNITY_2019) || defined(UNITY_2021)
        if (is_thread_attached()) {
            return std::invoke(std::forward<F>(func), std::forward<TArgs>(args)...);
        } else {
            return attached_invoke(std::forward<F>(func), std::forward<TArgs>(args)...);
        }
#else
        return attached_invoke(std::forward<F>(func), std::forward<TArgs>(args)...);
#endif
    }
}
struct il2cpp_thread : public std::thread {
    /// @brief Creates a thread that automatically will register with il2cpp and deregister once it exits.
    /// Ensure your arguments live longer than the thread, if they're by reference!
    /// @param pred The predicate to use for the thread
    /// @param args The arguments to pass to the thread (& predicate)
    /// @return created thread, which is the same as you creating a default one
    template <typename F, typename... TArgs>
    requires(std::is_invocable_v<F, std::decay_t<TArgs>...>)
    explicit il2cpp_thread(F&& pred, TArgs&&... args) :
        std::thread(&i2c::threading::attached_invoke<F, std::decay_t<TArgs>...>, std::forward<F>(pred), std::forward<TArgs>(args)...) {}

    // Default move ctor
    il2cpp_thread(il2cpp_thread&&) = default;

    // If joinable and destructed, join
    ~il2cpp_thread() {
        if (joinable()) {
            join();
        }
    }
};

template <typename F, typename... TArgs>
requires(std::is_invocable_v<F, TArgs...>)
inline std::future<std::invoke_result_t<F, TArgs...>> il2cpp_async(std::launch policy, F&& func, TArgs&&... args) {
    auto thread_func = &i2c::threading::il2cpp_async_internal<F, TArgs...>;
    return std::async<decltype(thread_func), F, TArgs...>(policy, std::move(thread_func), std::forward<F>(func), std::forward<TArgs>(args)...);
}

template <typename F, typename... TArgs>
requires(std::is_invocable_v<F, TArgs...>)
inline std::future<std::invoke_result_t<F, TArgs...>> il2cpp_async(F&& func, TArgs&&... args) {
    return il2cpp_async<F, TArgs...>(std::launch::any, std::forward<F>(func), std::forward<TArgs>(args)...);
}
