#pragma once

#include "types.hpp"

/// @brief An Array wrapper type that is responsible for holding an (ideally valid) pointer to an array on the GC heap.
/// Allows for C++ array semantics. Ex, [], begin(), end(), etc...
template <typename T>
struct ArrayW {
    using ptr = Array<T>*;
    using const_ptr = Array<T> const*;

    using value = T;
    using const_value = T const;
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T const&;

    using iterator = pointer;
    using const_iterator = const_pointer;

    /// @brief Default constructor wraps a nullptr array
    constexpr ArrayW() noexcept : val(nullptr) {}
    /// @brief Constructs an ArrayW that wraps a null value
    constexpr ArrayW(std::nullptr_t nptr) noexcept : val(nptr) {}
    /// @brief Create an ArrayW from an arbitrary pointer
    constexpr ArrayW(void* inst) noexcept : val(static_cast<ptr>(inst)) {}
    /// @brief Create an ArrayW from a pointer
    constexpr ArrayW(ptr inst) noexcept : val(inst) {}
    constexpr ArrayW(System::Array* inst) noexcept : val(inst) {}

    constexpr ArrayW(ArrayW const&) noexcept = default;
    constexpr ArrayW(ArrayW&&) noexcept = default;

    // Empty with size
    ArrayW(il2cpp_array_size_t size) {
        i2c::functions::initialize();
        val = reinterpret_cast<ptr>(i2c::functions::array_new(i2c::class_of<T>(), size));
    }
    // From initializer list
    template <typename U>
    requires(std::is_convertible_v<U, T>)
    ArrayW(std::initializer_list<U> vals) : ArrayW(vals.size()) {
        std::copy(vals.begin(), vals.end(), begin());
    }
    // From container
    template <typename U>
    requires(std::is_convertible_v<U, T>)
    explicit ArrayW(std::span<U> vals) : ArrayW(vals.size()) {
        std::copy(vals.begin(), vals.end(), begin());
    }
    // Convenience overload to convert vector to span
    template <typename U>
    requires(std::is_convertible_v<U, T>)
    explicit ArrayW(std::vector<U> const& vals) : ArrayW(std::span<U const>(vals)) {}

    constexpr bool operator==(ArrayW const&) const noexcept = default;

    constexpr ArrayW& operator=(ArrayW const&) noexcept = default;
    constexpr ArrayW& operator=(ArrayW&&) noexcept = default;

    constexpr ArrayW& operator=(ptr rhs) {
        this->val = rhs;
        return *this;
    }
    template <typename U>
    requires(std::is_convertible_v<U, T>)
    constexpr ArrayW& operator=(ArrayW<U> rhs) {
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

    [[nodiscard]] il2cpp_array_size_t size() const noexcept { return val->max_length; }
    bool empty() const noexcept { return size() == 0; }

    void assert_bounds(il2cpp_array_size_t i) const {
        if (i < 0 || i >= size()) {
            throw std::runtime_error(fmt::format("{} is out of bounds for array of length: {}", i, size()));
        }
    }

    iterator begin() { return val->_values; }
    const_iterator begin() const { return val->_values; }
    iterator end() { return val->_values + size(); }
    const_iterator end() const { return val->_values + size(); }

    auto rbegin() { return std::reverse_iterator(end()); }
    auto rbegin() const { return std::reverse_iterator(end()); }
    auto rend() { return std::reverse_iterator(begin()); }
    auto rend() const { return std::reverse_iterator(begin()); }

    reference operator[](il2cpp_array_size_t i) noexcept { return val->_values[i]; }
    const_reference operator[](il2cpp_array_size_t i) const noexcept { return val->_values[i]; }

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
        auto itr = std::find_if(begin(), end(), std::forward<TArgs>(args)...);
        if (itr == end()) {
            return {};
        }
        return *itr;
    }

    reference back() { return (*this)[size() - 1]; }
    const_reference back() const { return (*this)[size() - 1]; }
    reference back(auto&& pred) { return *rfind_if(pred); }
    const_reference back(auto&& pred) const { return *rfind_if(pred); }

    template <typename... TArgs>
    requires(std::is_default_constructible_v<value> && std::is_copy_constructible_v<value>)
    value back_or_default(TArgs&&... args) const {
        auto itr = std::find_if(rbegin(), rend(), std::forward<TArgs>(args)...);
        if (itr == rend()) {
            return {};
        }
        return *itr;
    }

    bool contains(const_reference item) const { return find(item) != end(); }

    void copy_to(std::span<value> destination, il2cpp_array_size_t index = 0) const {
        if (index + size() > destination.size()) {
            throw std::runtime_error("Destination span is too short for copy");
        }
        std::copy_n(begin(), size(), std::next(destination.begin(), index));
    }
    void copy_to(ArrayW<value> destination, il2cpp_array_size_t index = 0) const { copy_to(destination.ref_to(), index); }

    long index_of(const_reference item) const {
        auto itr = find(item);
        if (itr == end()) {
            return -1;
        }
        return std::distance(begin(), itr);
    }

    /// @brief Provides a reference span of the held data within this array. The span should NOT outlive this instance.
    /// @return The created span.
    std::span<value> ref_to() { return {val->_values, size()}; }
    /// @brief Provides a reference span of the held data within this array. The span should NOT outlive this instance.
    /// @return The created span.
    std::span<const_value> const ref_to() const { return {val->_values, size()}; }

#ifdef HAS_CODEGEN
    explicit constexpr operator ::System::Collections::ICollection*() noexcept { return static_cast<::System::Collections::ICollection*>(convert()); }
    explicit constexpr operator ::System::Collections::IEnumerable*() noexcept { return static_cast<::System::Collections::IEnumerable*>(convert()); }
    explicit constexpr operator ::System::Collections::IList*() noexcept { return static_cast<::System::Collections::IList*>(convert()); }
    explicit constexpr operator ::System::Collections::IStructuralComparable*() noexcept {
        return static_cast<::System::Collections::IStructuralComparable*>(convert());
    }
    explicit constexpr operator ::System::Collections::IStructuralEquatable*() noexcept {
        return static_cast<::System::Collections::IStructuralEquatable*>(convert());
    }
    explicit constexpr operator ::System::ICloneable*() noexcept { return static_cast<::System::ICloneable*>(convert()); }
#endif

   private:
    ptr val;
};

template <typename T>
struct i2c::type_check::no_arg_class<ArrayW<T>> {
    static inline Il2CppClass* get() { return no_arg_class<Array<T>*>::get(); }
};
MARK_GEN_REF_T(ArrayW);

static_assert(sizeof(ArrayW<int>) == sizeof(void*));
static_assert(i2c::type_check::wrapper_ref_type<ArrayW<int>>);

template <typename T, typename Char>
struct fmt::is_range<ArrayW<T>, Char> {
    static constexpr bool value = false;
};

template <typename T>
inline std::string format_as(ArrayW<T> array) {
    if (!array) {
        return fmt::format("ArrayW<{}>(null)", i2c::type_name<T>());
    }
    return fmt::format("{}", fmt::join(array.begin(), array.end(), ", "));
}
