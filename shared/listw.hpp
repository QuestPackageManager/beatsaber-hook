#pragma once

#include "arrayw.hpp"

#ifdef HAS_CODEGEN
#include "System/Collections/Generic/List_1.hpp"
#else
namespace System::Collections::Generic {
    template <typename T>
    struct List_1 : Il2CppObject {
        ArrayW<T> _items;
        int _size;
        int _version;
        Il2CppObject* _syncRoot;
    };
}

DEFINE_IL2CPP_GEN_CLASS(System::Collections::Generic::List_1, "System.Collections.Generic", "List`1");
MARK_GEN_REF_T(System::Collections::Generic::List_1);
#endif

template <typename T>
struct ListW {
    static_assert(i2c::type_check::valid_type<T>, "T must be a valid C# type!");

    using ptr = System::Collections::Generic::List_1<T>*;
    using const_ptr = System::Collections::Generic::List_1<T> const*;

    using value = T;
    using const_value = T const;
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T const&;

    using iterator = pointer;
    using const_iterator = const_pointer;

    /// @brief Default constructor wraps a nullptr array
    constexpr ListW() noexcept : val(nullptr) {}
    /// @brief Constructs an ListW that wraps a null value
    constexpr ListW(std::nullptr_t nptr) noexcept : val(nptr) {}
    /// @brief Create an ListW from an arbitrary pointer
    constexpr ListW(void* inst) noexcept : val(static_cast<ptr>(inst)) {}
    /// @brief Create an ListW from a pointer
    constexpr ListW(ptr inst) noexcept : val(inst) {}

    constexpr ListW(ListW const&) noexcept = default;
    constexpr ListW(ListW&&) noexcept = default;

    // Empty with size
    ListW(il2cpp_array_size_t size) {
        i2c::functions::initialize();
        val = reinterpret_cast<ptr>(i2c::functions::object_new(i2c::class_of<ptr>()));
        set_capacity(size);
    }
    // From initializer list
    template <typename U>
    requires(std::is_convertible_v<U, T>)
    ListW(std::initializer_list<U> vals) : ListW(vals.size()) {
        std::copy(vals.begin(), vals.end(), begin());
    }
    // From container
    template <typename U>
    requires(std::is_convertible_v<U, T>)
    explicit ListW(std::span<U const> vals) : ListW(vals.size()) {
        std::copy(vals.begin(), vals.end(), begin());
    }
    // Convenience overload to convert vector to span
    template <typename U>
    requires(std::is_convertible_v<U, T>)
    explicit ListW(std::vector<U> const& vals) : ListW(std::span<U const>(vals)) {}

    constexpr bool operator==(ListW const&) const noexcept = default;

    constexpr ListW& operator=(ListW const&) noexcept = default;
    constexpr ListW& operator=(ListW&&) noexcept = default;

    constexpr ListW& operator=(ptr rhs) noexcept {
        val = rhs;
        return *this;
    }
    template <typename U>
    requires(std::is_convertible_v<U, T>)
    constexpr ListW& operator=(ListW<U> rhs) noexcept {
        return operator=(static_cast<ptr>(rhs.val));
    }

    constexpr void* convert() const noexcept { return const_cast<void*>(static_cast<void*>(val)); }

    operator std::span<value>() { return ref_to(); }
    operator std::span<const_value> const() const { return ref_to(); }

    operator ptr() noexcept { return val; }
    operator const_ptr() const noexcept { return val; }

    ptr operator->() noexcept { return val; }
    const_ptr operator->() const noexcept { return val; }

    operator bool() const noexcept { return val != nullptr; }

    [[nodiscard]] inline il2cpp_array_size_t size() const noexcept { return val->_size; }
    inline bool empty() const noexcept { return size() == 0; }

    inline void assert_bounds(il2cpp_array_size_t i) const {
        if (i < 0 || i >= size()) {
            throw std::runtime_error(fmt::format("{} is out of bounds for list of length: {}", i, size()));
        }
    }

    iterator begin() { return val->_items.begin(); }
    const_iterator begin() const { return val->_items.begin(); }
    iterator end() { return val->_items.begin() + size(); }
    const_iterator end() const { return val->_items.begin() + size(); }

    auto rbegin() { return std::reverse_iterator(end()); }
    auto rbegin() const { return std::reverse_iterator(end()); }
    auto rend() { return std::reverse_iterator(begin()); }
    auto rend() const { return std::reverse_iterator(begin()); }

    reference operator[](il2cpp_array_size_t i) noexcept { return val->_items[i]; }
    const_reference operator[](il2cpp_array_size_t i) const noexcept { return val->_items[i]; }

    /// @brief Get a given index, performs bound checking and throws std::runtime_error on failure.
    /// @param i The index to get.
    /// @return The reference to the item.
    reference at(il2cpp_array_size_t i) {
        assert_bounds(i);
        return (*this)[i];
    }
    /// @brief Get a given index, performs bound checking and throws std::runtime_error on failure.
    /// @param i The index to get.
    /// @return The const reference to the item.
    const_reference at(il2cpp_array_size_t i) const {
        assert_bounds(i);
        return (*this)[i];
    }

    /// @brief Tries to get a given index, performs bound checking and returns a std::nullopt on failure.
    /// @param i The index to get.
    /// @return The reference_wrapper<T> to the item, mostly considered to be a T&.
    std::optional<std::reference_wrapper<value>> try_get(il2cpp_array_size_t i) noexcept {
        if (i < 0 || i >= size()) {
            return std::nullopt;
        }
        return (*this)[i];
    }
    /// @brief Tries to get a given index, performs bound checking and returns a std::nullopt on failure.
    /// @param i The index to get.
    /// @return The reference_wrapper<const T> to the item, mostly considered to be a const T&.
    std::optional<std::reference_wrapper<const_value>> try_get(il2cpp_array_size_t i) const noexcept {
        if (i < 0 || i >= size()) {
            return std::nullopt;
        }
        return (*this)[i];
    }

    iterator find(const_reference item) { return std::find(begin(), end(), item); }
    const_iterator find(const_reference item) const { return std::find(begin(), end(), item); }

    auto rfind(const_reference item) { return std::find(rbegin(), rend(), item); }
    auto rfind(const_reference item) const { return std::find(rbegin(), rend(), item); }

    iterator find_if(auto&& pred) { return std::find_if(begin(), end(), pred); }
    const_iterator find_if(auto&& pred) const { return std::find_if(begin(), end(), pred); }

    auto rfind_if(auto&& pred) { return std::find_if(rbegin(), rend(), pred); }
    auto rfind_if(auto&& pred) const { return std::find_if(rbegin(), rend(), pred); }

    reference front() { return (*this)[0]; }
    const_reference front() const { return (*this)[0]; }
    reference front(auto&& pred) { return *find_if(pred); }
    const_reference front(auto&& pred) const { return *find_if(pred); }

    template <typename... TArgs>
    requires(std::is_default_constructible_v<value> && std::is_copy_constructible_v<value>)
    value front_or_default(TArgs&&... args) const {
        auto itr = find_if(std::forward<TArgs>(args)...);
        if (itr == end()) {
            return {};
        }
        return *itr;
    }

    reference back() { return (*this)[size() - 1]; }
    const_reference back() const { return (*this)[size() - 1]; }
    reference back(auto&& pred) { return rfind_if(pred); }
    const_reference back(auto&& pred) const { return rfind_if(pred); }

    template <typename... TArgs>
    requires(std::is_default_constructible_v<value> && std::is_copy_constructible_v<value>)
    value back_or_default(TArgs&&... args) const {
        auto itr = rfind_if(std::forward<TArgs>(args)...);
        if (itr == rend()) {
            return {};
        }
        return *itr;
    }

    bool contains(const_reference item) const { return find(item) != end(); }

    void copy_to(std::span<value> destination, int index) const {
        if (index + size() > destination.size()) {
            throw std::runtime_error("Destination span is too short for copy");
        }
        std::copy(begin(), end(), std::next(destination.begin(), index));
    }

    long index_of(const_reference item) const {
        auto itr = find(item);
        if (itr == end()) {
            return -1;
        }
        return std::distance(begin(), itr);
    }

    void clear() {
        val->_version++;
        if constexpr (i2c::type_check::ref_type<T>) {
            if (size() > 0) {
                std::fill(begin(), end(), T{});
            }
        }
        val->_size = 0;
    }

    void insert_at(il2cpp_array_size_t index, value&& item) {
        if (index > size()) {
            throw std::runtime_error(val, "Capacity size too small");
        }
        if (size() == val->_items.size()) {
            ensure_capacity(size() + 1);
        }
        if (index < size()) {
            std::copy_backward(begin() + index, end(), end() + 1);
        }
        val->_version++;
        (*this)[index] = std::move(item);
        val->_size++;
    }

    void push_back(value&& item) {
        val->_version++;
        ensure_capacity(size() + 1);
        *end() = std::move(item);
        val->_size += 1;
    }

    template <typename... TArgs>
    void emplace_back(TArgs&&... args) {
        val->_version++;
        ensure_capacity(size() + 1);
        if constexpr (requires { std::remove_pointer_t<value>::New_ctor(std::forward(args)...); }) {
            *end() = std::remove_pointer_t<value>::New_ctor(std::forward(args)...);
        } else {
            std::construct_at<value>(end(), std::forward(args)...);
        }
        val->_size += 1;
    }

    bool erase(value item) {
        auto index = index_of(item);
        if (index < 0) {
            return false;
        }
        erase_at(index.value());
        return true;
    }

    void erase_at(il2cpp_array_size_t index) {
        if (index >= size()) {
            throw std::runtime_error("Erased index is greater than size");
        }
        val->_version++;
        if (index < size()) {
            std::copy(begin() + index + 1, end(), begin() + index);
        }
        if constexpr (i2c::type_check::ref_type<T>) {
            *end() = T{};
        }
        val->_size--;
    }

    void erase_range(int index, int count) {
        if (count <= 0) {
            return;
        }
        if (index < 0) {
            throw std::runtime_error("First index of erased range is less than 0");
        }
        if (size() - index < count) {
            throw std::runtime_error("Erased range is too large");
        }
        val->_version++;
        if (index < size()) {
            std::copy(begin() + index + count, end(), begin() + index);
        }
        val->_size -= count;
        if constexpr (i2c::type_check::ref_type<T>) {
            std::fill(end(), end() + count, T{});
        }
    }

    /// @brief Adds a collection to the end of the list.
    void insert_range(auto&& range_begin, auto&& range_end) {
        auto range_size = std::distance(range_begin, range_end);
        if (range_size == 0) {
            return;
        }
        ensure_capacity(size() + range_size);
        std::copy(range_begin, range_end, end());
        val->_size += range_size;
        val->_version++;
    }
    void insert_range(auto&& begin, int count) { insert_range(begin, begin + count); }
    void insert_range(std::span<T const> span) { insert_range(span.begin(), span.end()); }

    /// @brief Provides a reference span of the held data within this list. The span should NOT outlive this instance.
    /// @return The created span.
    std::span<T> ref_to() { return std::span(begin(), end()); }

    /// @brief Provides a reference span of the held data within this list. The span should NOT outlive this instance.
    /// @return The created span.
    std::span<T const> ref_to() const { return std::span(begin(), end()); }

    ArrayW<value> to_array() const { return ArrayW<value>(ref_to()); }

   protected:
    void ensure_capacity(il2cpp_array_size_t min) {
        if (val->_items.size() >= min) {
            return;
        }
        auto num = val->_items.size() * 2;
        if (num == 0) {
            num = 4;
        } else if (num > 2146435071) {
            num = 2146435071;
        }
        if (num < min) {
            num = min;
        }
        set_capacity(num);
    }

    void set_capacity(il2cpp_array_size_t value) {
        if (value < size()) {
            throw std::runtime_error("Attempting to set list capacity to smaller than its size");
        }
        if (value == val->_items.size()) {
            return;
        }
        auto array = ArrayW<T>(value);
        if (size() > 0) {
            std::copy(begin(), end(), array.begin());
        }
        val->_items = array;
    }

    ptr val;
};

template <typename T>
struct i2c::type_check::no_arg_class<ListW<T>> {
    static inline Il2CppClass* get() { return no_arg_class<System::Collections::Generic::List_1<T>*>::get(); }
};
MARK_GEN_REF_T(ListW);

static_assert(sizeof(ListW<int>) == sizeof(void*));
static_assert(i2c::type_check::wrapper_ref_type<ListW<int>>);

template <typename T, typename Char>
struct fmt::is_range<ListW<T>, Char> {
    static constexpr bool value = false;
};

template <typename T>
inline std::string format_as(ListW<T> list) {
    if (!list) {
        return fmt::format("ListW<{}>(null)", i2c::type_name<T>());
    }
    return fmt::format("{}", fmt::join(list.begin(), list.end(), ", "));
}
