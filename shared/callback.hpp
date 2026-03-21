#pragma once

#include <set>
#include <unordered_set>

namespace detail {
    template <template <typename...> typename C, typename T>
    concept valid_container = requires(C<T> container, T item) {
        container.erase(item);
        container.emplace(item);
        container.clear();
        container.begin();
        container.end();
        container.size();
    };

    template <class T>
    struct abstract_func;

    template <typename R, typename T, typename... TArgs>
    struct abstract_func<R(T*, TArgs...)> {
        virtual ~abstract_func() = default;

        virtual T* instance() const = 0;
        virtual void* ptr() const = 0;

        virtual R operator()(TArgs... args) const noexcept = 0;
    };

    template <class T>
    struct func_wrapper;

    template <typename R, typename... TArgs>
    struct func_wrapper<R (*)(TArgs...)> : abstract_func<R(void*, TArgs...)> {
        func_wrapper(auto&& f) : held(f) {}

        R operator()(TArgs... args) const noexcept override {
            if constexpr (std::is_same_v<R, void>) {
                held(args...);
            } else {
                return held(args...);
            }
        }

        void* instance() const override { return nullptr; }
        void* ptr() const override { return reinterpret_cast<void*>(held); }

        R (*held)(TArgs...);
    };

    template <typename R, typename T, typename... TArgs>
    struct func_wrapper<R (T::*)(TArgs...)> : abstract_func<R(void*, TArgs...)> {
        func_wrapper(auto&& f, T* inst) : held(f), _instance(inst) {}

        void* instance() const override { return _instance; }
        void* ptr() const override {
            using fptr = R (T::*)(TArgs...);
            union dat {
                fptr wrapper;
                void* data;
            };
            dat d{.wrapper = held};
            return d.data;
        }

        R operator()(TArgs... args) const noexcept override {
            if constexpr (std::is_same_v<R, void>) {
                (reinterpret_cast<T*>(_instance)->*held)(args...);
            } else {
                return (reinterpret_cast<T*>(_instance)->*held)(args...);
            }
        }

        R (T::*held)(TArgs...);
        T* _instance;
    };

    template <typename R, typename... TArgs>
    struct func_wrapper<std::function<R(TArgs...)>> : abstract_func<R(void*, TArgs...)> {
        func_wrapper(std::function<R(TArgs...)> const& f) : held(f), handle(const_cast<void*>(reinterpret_cast<void const*>(&f))) {}

        [[nodiscard]] void* instance() const override { return nullptr; }
        [[nodiscard]] void* ptr() const override { return handle; }

        R operator()(TArgs... args) const noexcept override {
            if constexpr (std::is_same_v<R, void>) {
                held(args...);
            } else {
                return held(args...);
            }
        }

        std::function<R(TArgs...)> const held;
        void* handle;
    };

    template <typename R, typename T, typename... TArgs>
    bool operator==(abstract_func<R(T*, TArgs...)> const& a, abstract_func<R(T*, TArgs...)> const& b) {
        return a.instance() == b.instance() && a.ptr() == b.ptr();
    }

    template <typename R, typename T, typename... TArgs>
    bool operator<(abstract_func<R(T*, TArgs...)> const& a, abstract_func<R(T*, TArgs...)> const& b) {
        return a.ptr() < b.ptr();
    }

    template <class T>
    struct thin_virtual_layer;

    template <typename R, typename T, typename... TArgs>
    struct thin_virtual_layer<R(T*, TArgs...)> {
        thin_virtual_layer(R (*ptr)(TArgs...)) : func(new func_wrapper<R (*)(TArgs...)>(ptr)) {}
        thin_virtual_layer(auto&& f) : func(new func_wrapper<std::function<R(TArgs...)>>(f)) {}
        template <typename Q>
        thin_virtual_layer(auto&& f, Q* inst) : func(new func_wrapper<R (Q::*)(TArgs...)>(f, inst)) {}

        void* instance() const { return func->instance(); }
        void* ptr() const { return func->ptr(); }

        R operator()(TArgs... args) const noexcept { (*func)(args...); }
        bool operator==(thin_virtual_layer<R(T*, TArgs...)> const other) const { return *func == *other.func; }
        bool operator<(thin_virtual_layer<R(T*, TArgs...)> const other) const { return *func < *other.func; }

        friend struct std::hash<thin_virtual_layer<R(T*, TArgs...)>>;

       private:
        std::shared_ptr<abstract_func<R(T*, TArgs...)>> func;
    };
}

template <typename R, typename T, typename... TArgs>
struct std::hash<detail::abstract_func<R(T*, TArgs...)>> {
    size_t operator()(detail::abstract_func<R(T*, TArgs...)> const& obj) const noexcept {
        auto seed = std::hash<void*>{}(obj.instance());
        return seed ^ std::hash<void*>{}(reinterpret_cast<void*>(obj.ptr())) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

template <typename R, typename T, typename... TArgs>
struct std::hash<detail::thin_virtual_layer<R(T*, TArgs...)>> {
    size_t operator()(detail::thin_virtual_layer<R(T*, TArgs...)> const& obj) const noexcept {
        return std::hash<detail::abstract_func<R(T*, TArgs...)>>{}(*obj.func);
    }
};

// TODO: Make a version of this for C# delegates?
// TODO: Also require the function type to be invokable and all that
template <template <typename...> typename C, typename... TArgs>
requires(detail::valid_container<C, detail::thin_virtual_layer<void(void*, TArgs...)>>)
class basic_event_callback {
    void invoke(TArgs... args) const {
#ifndef NO_EVENT_CALLBACK_INVOKE_SAFETY
        // copy the callbacks so an unsubscribe during invoke of the container doesn't cause UB
        auto to_invoke = callbacks;
#else
        auto& to_invoke = callbacks;
#endif
        for (auto& callback : to_invoke) {
            callback(args...);
        }
    }

    auto size() const { return callbacks.size(); }
    void clear() { callbacks.clear(); }

    // The instance provide here should have lifetime > calls to invoke.
    // If the provided instance dies before this instance, or before invoke is called, invoke will crash.
    template <typename T>
    void add(void (T::*callback)(TArgs...), T* inst) {
        callbacks.emplace(callback, inst);
    }
    void add(void (*callback)(TArgs...)) { callbacks.emplace(callback); }
    void add(detail::thin_virtual_layer<void(void*, TArgs...)> callback) { callbacks.emplace(std::move(callback)); }

    template <typename T>
    void remove(void (T::*callback)(TArgs...)) {
        // Removal of member functions is expensive because we need to remove all member functions regardless of instance
        for (auto itr = callbacks.begin(); itr != callbacks.end();) {
            union dat {
                decltype(callback) wrapper;
                void* data;
            };
            dat d{.wrapper = callback};
            if (itr->ptr() == d.data) {
                itr = callbacks.erase(itr);
            } else {
                ++itr;
            }
        }
    }
    void remove(void (*callback)(TArgs...)) { callbacks.erase(callback); }
    void remove(detail::thin_virtual_layer<void(void*, TArgs...)> callback) { callbacks.erase(callback); }

    basic_event_callback& operator+=(auto callback) {
        add(callback);
        return *this;
    }
    basic_event_callback& operator-=(auto callback) {
        remove(callback);
        return *this;
    }

   private:
    C<detail::thin_virtual_layer<void(void*, TArgs...)>> callbacks;
};

// Good default for most
template <typename... TArgs>
using event_callback = basic_event_callback<std::set, TArgs...>;

template <typename... TArgs>
using unordered_event_callback = basic_event_callback<std::unordered_set, TArgs...>;
