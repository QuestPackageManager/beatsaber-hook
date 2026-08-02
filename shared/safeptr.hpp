#pragma once

#include "api.hpp"
#include "exceptions.hpp"
#include "types.hpp"

namespace i2c::detail {
    /// @brief Adds to the reference count of an address. If the address does not exist, initializes a new entry for it to 1.
    /// @param addr The address to add.
    void add_count(void* addr);
    /// @brief Decreases the reference count of an address. If the address has 1 or fewer references, erases it.
    /// @param addr The address to decrease.
    void remove_count(void* addr);
    /// @brief Gets the reference count of an address, or 0 if no such address exists.
    /// @param addr The address to get the count of.
    /// @return The reference count of the provided address.
    size_t get_count(void* addr);

    /// @brief Represents a smart pointer that has a reference count, which does NOT destroy the held instance on refcount reaching 0.
    /// @tparam T The type to wrap as a pointer.
    template <typename T>
    struct count_ptr {
        /// @brief Default constructor for Count Pointer, defaults to a nullptr, with 0 references.
        explicit count_ptr() noexcept : ptr(nullptr) {}
        /// @brief Construct a count pointer from the provided pointer, adding to the reference count (if non-null) for the provided pointer.
        /// @param p The pointer to provide. May be null, which does nothing.
        explicit count_ptr(T* p) : ptr(p) {
            if (p) {
                add_count(p);
            }
        }
        /// @brief Copy constructor, copies and adds to the reference count for the held non-null pointer.
        count_ptr(count_ptr<T> const& other) : ptr(other.ptr) {
            if (ptr) {
                add_count(ptr);
            }
        }
        /// @brief Move constructor moves the pointer and keeps the reference count the same.
        count_ptr(count_ptr&& other) noexcept {
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        /// @brief Copy assignment, emplaces the held pointer from the other instance, updating reference counts as necessary.
        count_ptr& operator=(count_ptr const& other) {
            emplace(other.ptr);
            return *this;
        }
        /// @brief Move assignment, moves the pointer and keeps the reference count the same.
        count_ptr& operator=(count_ptr&& other) {
            if (this != &other) {
                if (ptr) {
                    remove_count(ptr);
                }
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }
        /// @brief Destructor, decreases the ref count for the held non-null pointer.
        ~count_ptr() {
            if (ptr) {
                remove_count(ptr);
            }
        }
        /// @brief Gets the reference count held by this pointer.
        /// @return The reference count for this pointer, or 0 if the held pointer is null.
        size_t count() const {
            if (ptr) {
                return get_count(ptr);
            }
            return 0;
        }
        /// @brief Emplaces a new pointer into the shared pointer, decreasing the existing ref count as necessary.
        /// @param val The new pointer to replace the currently held one with.
        inline void emplace(T* val) {
            if (val == ptr) {
                return;
            }
            if (ptr) {
                remove_count(ptr);
            }
            ptr = val;
            if (ptr) {
                add_count(ptr);
            }
        }

        /// @brief Get the raw pointer. Should ALMOST NEVER BE USED, UNLESS SCOPE GUARANTEES IT DIES BEFORE THIS INSTANCE DOES!
        /// @return The raw pointer saved by this instance.
        constexpr T* const get() const noexcept { return ptr; }

        count_ptr& operator=(T* val) {
            emplace(val);
            return *this;
        }

        T& operator*() noexcept { return *ptr; }
        T const& operator*() const noexcept { return *ptr; }
        T* operator->() noexcept { return ptr; }
        T* const operator->() const noexcept { return ptr; }

        constexpr operator bool() noexcept { return ptr != nullptr; }
        constexpr operator bool() const noexcept { return ptr != nullptr; }

       private:
        T* ptr;
    };
}

#ifdef HAS_CODEGEN
namespace UnityEngine {
    class Object;
}
namespace i2c::detail {
    template <typename T, bool U = false>
    constexpr bool unity_guess = std::derived_from<std::remove_pointer_t<T>, UnityEngine::Object>;
}
#else
namespace i2c::detail {
    template <typename T, bool U = false>
    constexpr bool unity_guess = U;
}
#endif

/// @brief Represents a C++ type that wraps a C# pointer, keeping it valid for the entire lifetime of this instance.
/// Note that Unity objects can still be destroyed, even if their C# pointer remains valid.
/// Instances must be created at a time such that i2c::functions::initialize is valid for all non-default constructors.
/// @tparam T The type of the instance to wrap (without a pointer).
/// @tparam U Explicitly specify if the wrapped type is a Unity object.
template <typename T, bool U = i2c::detail::unity_guess<T>>
requires(i2c::type_check::ref_type<T>)
struct safe_ptr {
    /// @brief Default constructor. Should be paired with emplace or = to ensure validity.
    safe_ptr() = default;
    /// @brief Copy constructor copies the HANDLE, that is, the held pointer remains the same.
    /// Note that this means if you modify one safe_ptr's held instance, all others that point to the same location will also reflect this change.
    /// Has a small performance overhead due to updating the reference count.
    safe_ptr(safe_ptr const& other) : handle(other.handle) {}
    /// @brief Default move constructor. Moves the internal handle and keeps reference count the same.
    safe_ptr(safe_ptr&& other) = default;
    /// @brief Construct a safe_ptr<T> with the provided instance pointer or wrapper (which may be null).
    /// If you wish to wrap a non-existent pointer (ex, use as a default constructor) see the 0 arg constructor instead.
    safe_ptr(T const& inst) : handle(wrapper::allocate(inst)) {}
    /// @brief Destructor. Destroys the internal wrapper type, if necessary.
    ~safe_ptr() { clear(); }

    /// @brief Destroys the internal wrapper type, if necessary.
    /// Aborts if a wrapper type exists and must be freed, yet GC_free does not exist.
    inline void clear() {
        // Clearing up without an internal handle is trivial
        if (!handle) {
            return;
        }
        // If our internal handle has 1 instance, we need to clean up the instance it points to.
        // Otherwise, some other safe_ptr is currently holding a reference to this instance, so keep it around.
        if (handle.count() <= 1) {
            i2c::functions::initialize();
            if (!i2c::functions::has_gc_funcs) {
                throw i2c::trace_exception("A safe_ptr<T> instance was created too early or a necessary GC function was not found!");
            }
            i2c::functions::gc_free_fixed(handle.get());
        }
        // ensure we don't try to clear the same handle twice
        handle = nullptr;
    }

    /// @brief Emplace a new value into this safe_ptr, freeing an existing one, if it exists.
    /// @param other The instance to emplace.
    inline void emplace(T const& other) {
        clear();
        handle = wrapper::allocate(other);
    }

    inline safe_ptr& operator=(T const& other) {
        emplace(other);
        return *this;
    }
    inline safe_ptr& operator=(safe_ptr const& other) {
        handle = other.handle;
        return *this;
    }

    constexpr void* convert() const noexcept { return const_cast<void*>(handle->inst); }

    /// @brief Performs an il2cpp type checked cast from T to U.
    /// This function will throw an exception if the cast fails. i2c::result<T2> can be used to capture errors instead.
    /// @tparam T2 The type to cast to.
    /// @tparam U2 Explicitly specify if the casted safe_ptr is a unity object.
    /// @return A new safe_ptr of the cast value.
    template <typename T2, bool U2 = i2c::detail::unity_guess<T2, U>>
    requires(i2c::type_check::has_class<i2c::remove_result_t<T2>>)
    [[nodiscard]] inline auto cast() const noexcept(i2c::is_result_v<T2>) {
        using R = i2c::change_result_t<T2, safe_ptr<i2c::remove_result_t<T2>, U2>>;
        if (!(*this)) {
            return i2c::result_or_throw<R>("A safe_ptr<T> instance is holding a null handle!");
        }
        auto* k1 = i2c::class_of<i2c::remove_result_t<T2>>();
        auto* k2 = *reinterpret_cast<Il2CppClass**>(handle->inst);
        if (!k1 || !k2) {
            return i2c::result_or_throw<R>("Invalid class in safe_ptr cast!");
        }
        i2c::functions::initialize();
        if (k1 == k2 || i2c::functions::class_is_assignable_from(k1, k2)) {
            if constexpr (i2c::type_check::wrapper_type<i2c::remove_result_t<T2>>) {
                return R(i2c::remove_result_t<T2>(handle->inst));
            } else {
                return R(reinterpret_cast<i2c::remove_result_t<T2>>(handle->inst));
            }
        }
        return i2c::result_or_throw<R>("The type could not be cast safely! Check your safe_ptr/count_ptr cast calls!");
    }

    /// @brief Performs an il2cpp type checked cast from T to U, returning a default constructed safe_ptr if it fails.
    /// @tparam T2 The type to cast to.
    /// @tparam U2 Explicitly specify if the casted safe_ptr is a unity object.
    /// @return A new safe_ptr of the cast value, if successful.
    template <typename T2, bool U2 = i2c::detail::unity_guess<T2, U>>
    [[nodiscard]] inline safe_ptr<T2, U2> try_cast() const noexcept {
        return cast<i2c::result<T2>, U2>().value_or(safe_ptr<T2, U2>{});
    }

    T ptr() const {
        if (!handle) {
            throw i2c::trace_exception("A safe_ptr<T> instance is holding a null handle!");
        }
        if constexpr (i2c::type_check::wrapper_type<T>) {
            return T(handle->inst);
        } else {
            return reinterpret_cast<T>(handle->inst);
        }
    }

    /// @brief Returns false if this is a defaultly constructed safe_ptr or if the held pointer evaluates to false.
    operator bool() const noexcept {
        if (!handle || !handle->inst) {
            return false;
        }
        // If Unity, check m_CachedPtr, which is the first field in UnityEngine.Object with an offset of 0x10
        if (U && !*reinterpret_cast<void* const*>(reinterpret_cast<char const*>(convert()) + 0x10)) {
            return false;
        }
        return true;
    }

    template <typename T2, bool U2>
    requires(std::is_convertible_v<T2, T> || std::is_same_v<T2, T>)
    bool operator==(safe_ptr<T2, U2> const& other) const {
        if (!other || !(*this)) {
            return static_cast<bool>(other) == static_cast<bool>(*this);
        }
        return other.convert() == convert();
    }

    template <typename T2 = T>
    requires(std::is_convertible_v<T2, T> || std::is_same_v<T2, T>)
    bool operator==(T2 const other) const {
        if (!other || !(*this)) {
            return static_cast<bool>(other) == static_cast<bool>(*this);
        }
        if constexpr (i2c::type_check::wrapper_type<T2>) {
            return other.convert() == convert();
        } else {
            return reinterpret_cast<void const* const>(other) == handle->inst;
        }
    }

    /// @brief Dereferences the instance pointer to a reference type of the held instance.
    [[nodiscard]] T operator->() { return ptr(); }
    [[nodiscard]] T const operator->() const { return ptr(); }
    /// @brief Explicitly cast this instance to a T.
    /// Note, however, that the lifetime of this returned T is not longer than the lifetime of this instance.
    /// Consider passing a safe_ptr reference or copy instead.
    [[nodiscard]] explicit operator T const() const { return ptr(); }

   private:
    struct wrapper {
        // Must be explicitly GC freed and allocated
        wrapper() = delete;
        ~wrapper() = delete;

        static wrapper* allocate(T const& instance) {
            // It should be safe to assume that gc_alloc_fixed returns a non-null pointer. If it does return null, we have a pretty big issue.
            i2c::functions::initialize();
            if (!i2c::functions::has_gc_funcs) {
                throw i2c::trace_exception("A safe_ptr<T> instance was created too early or a necessary GC function was not found!");
            }
            auto allocated = CRASH_UNLESS(reinterpret_cast<wrapper*>(i2c::functions::gc_alloc_fixed(sizeof(wrapper))));
            if constexpr (i2c::type_check::wrapper_type<T>) {
                allocated->inst = instance.convert();
            } else {
                allocated->inst = reinterpret_cast<void*>(instance);
            }
            return allocated;
        }
        void* inst;
    };

    i2c::detail::count_ptr<wrapper> handle;
};
