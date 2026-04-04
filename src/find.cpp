#include "find.hpp"

#include "debug.hpp"

#include <shared_mutex>

// Allow transparent comparisons and conversions between a span and a vector
template <typename T>
struct span_vec_w : i2c::view<T> {
    using base_type = i2c::view<T>;
    using base_type::base_type;

    constexpr span_vec_w() = default;
    constexpr span_vec_w(span_vec_w const&) = default;
    constexpr span_vec_w(span_vec_w&&) = default;

    constexpr span_vec_w(i2c::view<T> view) : base_type(std::move(view)) {}

    constexpr span_vec_w& operator=(span_vec_w const&) = default;
    constexpr span_vec_w& operator=(span_vec_w&&) = default;

    operator std::vector<T>() { return {this->begin(), this->end()}; }

    bool operator==(std::vector<T> const& rhs) const {
        if (this->size() != rhs.size()) {
            return false;
        }
        for (decltype(this->size()) i = 0; i < this->size(); i++) {
            if ((*this)[i] != rhs[i]) {
                return false;
            }
        }
        return true;
    }
};

template <typename T>
struct std::hash<span_vec_w<T>> {
    std::size_t operator()(span_vec_w<T> const& val) const noexcept {
        std::size_t seed = val.size();
        for (auto const& i : val) {
            seed = i2c::hash_combine(i, seed);
        }
        return seed;
    }
};

template <typename T>
struct std::hash<std::vector<T>> {
    std::size_t operator()(std::vector<T> const& val) const noexcept { return std::hash<span_vec_w<T>>{}(val); }
};

// Used to hash tuples used as keys in caches
struct tup_hash {
    template <size_t I = 0, typename... TArgs>
    size_t recursive_hash(std::tuple<TArgs...> const& tup, size_t current = 0) const noexcept {
        current = i2c::hash_combine(std::get<I>(tup), current);
        if constexpr (I < sizeof...(TArgs) - 1) {
            return recursive_hash<I + 1>(tup, current);
        } else {
            return current;
        }
    }
    template <typename... TArgs>
    size_t operator()(std::tuple<TArgs...> const& tup) const noexcept {
        return recursive_hash(tup);
    }
    using is_transparent = std::true_type;
};

// Used to compare tuples (potentially of different types) used as keys in caches
struct tup_eq {
    template <size_t I = 0, typename... T1Args, typename... T2Args>
    bool recursive_equal(std::tuple<T1Args...> const& lhs, std::tuple<T2Args...> const& rhs) const {
        bool equals = std::get<I>(lhs) == std::get<I>(rhs);
        if constexpr (I < sizeof...(T1Args) - 1) {
            return equals && recursive_equal<I + 1>(lhs, rhs);
        } else {
            return equals;
        }
    }
    template <typename... T1Args, typename... T2Args>
    requires(sizeof...(T1Args) == sizeof...(T2Args))
    bool operator()(std::tuple<T1Args...> const& lhs, std::tuple<T2Args...> const& rhs) const {
        return recursive_equal(lhs, rhs);
    }
    using is_transparent = std::true_type;
};

template <typename K, typename V>
using tuple_map = std::unordered_map<K, V, tup_hash, tup_eq>;

Il2CppClass* i2c::find_class(find_class_info const& info) noexcept {
    if (auto by_name = std::get_if<find_class_info::by_name>(&info.data)) {
        return get_class_from_name(by_name->namespaze, by_name->name);
    }
    if (auto by_inst = std::get_if<find_class_info::by_instance>(&info.data)) {
        return by_inst->instance->klass;
    }
    if (auto klass = std::get_if<Il2CppClass*>(&info.data)) {
        return *klass;
    }
    return nullptr;
}

tuple_map<std::tuple<Il2CppClass*, std::string, int>, MethodInfo const*> args_cache;
tuple_map<std::tuple<Il2CppClass*, std::string, std::vector<Il2CppClass const*>, std::vector<Il2CppType const*>>, MethodInfo const*> types_cache;
std::shared_mutex caches_lock;

static MethodInfo const* find_method(Il2CppClass* klass, std::function<std::pair<bool, int>(MethodInfo const*)> const& match) {
    int best_val = std::numeric_limits<int>::max();
    MethodInfo const* best_match = nullptr;
    bool multiple = false;

    // Iterate through class and its parents
    while (klass) {
        if (!klass->initialized_and_no_error) {
            i2c::functions::Class_Init(klass);
        }
        // Check all methods against the provided match function
        for (auto method : std::span(klass->methods, klass->method_count)) {
            auto [exact_match, match_value] = match(method);
            if (exact_match) {
                return method;
            } else if (match_value < best_val) {
                multiple = best_match;
                best_val = match_value;
                best_match = method;
            }
        }
        klass = klass->parent;
    }

    if (multiple) {
        i2c::logger.warn("Found multiple methods that match! With weight {}, using {}:", best_val, fmt::ptr(best_match));
        i2c::log_method(i2c::logger, best_match);
    }
    return best_match;
}

template <typename... TArgs, typename... KArgs>
static MethodInfo const* find_method_cached(
    Il2CppClass* klass,
    std::function<std::pair<bool, int>(MethodInfo const*)> const& match,
    tuple_map<std::tuple<Il2CppClass*, TArgs...>, MethodInfo const*>& cache,
    KArgs... cache_key
) {
    RET_DEF_UNLESS(i2c::logger, klass);

    std::shared_lock shared_lock(caches_lock);
    auto itr = cache.find(std::make_tuple(klass, cache_key...));
    if (itr != cache.end()) {
        return itr->second;
    }
    shared_lock.unlock();

    i2c::functions::initialize();
    auto found = find_method(klass, match);
    if (!found) {
        i2c::logger.error("Could not find method {} in class {}!", std::make_tuple(cache_key...), i2c::class_standard_name(klass));
        i2c::log_methods(i2c::logger, klass, true);
        // Cache it anyway - good chance of it being attempted again, if we don't crash
    }

    std::unique_lock unique_lock(caches_lock);
    cache[std::tuple<Il2CppClass*, TArgs...>(klass, cache_key...)] = found;
    return found;
}

// For lazy error formatting in find_method
std::string format_as(std::tuple<std::string_view, int> const& tup) {
    auto& [name, args] = tup;
    if (args >= 0) {
        return fmt::format("{}({} args)", name, args);
    } else {
        return std::string(name);
    }
}
std::string format_as(std::tuple<std::string_view, span_vec_w<Il2CppClass const*>, span_vec_w<Il2CppType const*>> const& tup) {
    auto& [name, gens, args] = tup;

    std::vector<std::string> arg_type_names;
    arg_type_names.reserve(args.size());
    for (auto t : args) {
        arg_type_names.emplace_back(i2c::type_simple_name(t));
    }
    if (gens.empty()) {
        return fmt::format("{}({})", name, fmt::join(arg_type_names, ", "));
    }

    std::vector<std::string> gen_type_names;
    gen_type_names.reserve(gens.size());
    for (auto t : gens) {
        gen_type_names.emplace_back(i2c::class_standard_name(t));
    }
    return fmt::format("{}<{}>({})", name, fmt::join(gen_type_names, ", "), fmt::join(arg_type_names, ", "));
}

static int param_distance(Il2CppClass* method_class, Il2CppClass* passed_class) {
    if (!method_class || !passed_class) {
        return 1;
    }
    if (method_class == passed_class) {
        return 0;
    }

    int distance = 0;

    bool is_method_iface = method_class->flags & TYPE_ATTRIBUTE_INTERFACE;
    bool is_passed_iface = passed_class->flags & TYPE_ATTRIBUTE_INTERFACE;
    // method is an interface, we gave a concrete type, avoid
    if (is_passed_iface && !is_method_iface) {
        return 1000;
    }
    // if method is interface, just add lots of weight so we choose a concrete type instead if possible
    if (is_method_iface) {
        distance += 5;
    }

    // add distance the farther away the types are in inheritance (should this affect the interface check?)
    while (passed_class && passed_class != method_class) {
        if (!i2c::functions::class_is_assignable_from(method_class, passed_class)) {
            break;
        }
        passed_class = passed_class->parent;
        distance++;
    }

    // find all interfaces which intersect with our expected type
    std::span<Il2CppClass const* const> method_ifaces = {method_class->implementedInterfaces, method_class->interfaces_count};
    std::span<Il2CppClass const* const> passed_ifaces = {passed_class->implementedInterfaces, passed_class->interfaces_count};
    // subtract distance by specifity of interface since it allows specifity
    size_t method_i = 0;
    size_t passed_i = 0;
    // effectively set_intersection but just a number instead of an iterator output
    while (method_i < method_ifaces.size() && passed_i < passed_ifaces.size()) {
        if (method_ifaces[method_i] < passed_ifaces[passed_i]) {
            method_i++;
        } else if (method_ifaces[method_i] > passed_ifaces[passed_i]) {
            passed_i++;
        } else {
            method_i++;
            passed_i++;
            distance--;
        }
    }

    return distance;
}

MethodInfo const* i2c::find_method(find_class_info const& class_info, find_method_info const& info) noexcept {
    if (auto by_name = std::get_if<find_method_info::by_name>(&info.data)) {
        return find_method_cached(
            find_class(class_info),
            [&by_name](MethodInfo const* method) {
                if (by_name->name == method->name) {
                    return std::make_pair(true, std::numeric_limits<int>::max());
                }
                return std::make_pair(false, std::numeric_limits<int>::max());
            },
            args_cache,
            by_name->name,
            -1
        );
    }
    if (auto by_args = std::get_if<find_method_info::by_args>(&info.data)) {
        return find_method_cached(
            find_class(class_info),
            [&by_args](MethodInfo const* method) {
                if (by_args->name == method->name && by_args->args == method->parameters_count) {
                    return std::make_pair(true, std::numeric_limits<int>::max());
                }
                return std::make_pair(false, std::numeric_limits<int>::max());
            },
            args_cache,
            by_args->name,
            by_args->args
        );
    }
    if (auto by_types = std::get_if<find_method_info::by_types>(&info.data)) {
        return find_method_cached(
            find_class(class_info),
            [&by_types](MethodInfo const* method) {
                if (by_types->name != method->name) {
                    return std::make_pair(false, std::numeric_limits<int>::max());
                }
                auto [matches, exact] = param_match(method, by_types->generics, by_types->params);
                if (!matches) {
                    return std::make_pair(false, std::numeric_limits<int>::max());
                }
                if (exact) {
                    return std::make_pair(true, std::numeric_limits<int>::max());
                }
                // Is this overload resolution even necessay?
                int weight = 0;
                for (size_t i = 0; i < by_types->params.size(); i++) {
                    auto method_class = i2c::functions::type_get_class_or_element_class(method->parameters[i]);
                    auto passed_class = i2c::functions::type_get_class_or_element_class(by_types->params[i]);
                    weight += param_distance(method_class, passed_class);
                }
                return std::make_pair(false, weight);
            },
            types_cache,
            by_types->name,
            span_vec_w(by_types->generics),
            span_vec_w(by_types->params)
        );
    }
    // Should the slot finds also be cached?
    if (auto by_slot = std::get_if<find_method_info::by_slot>(&info.data)) {
        auto klass = RET_DEF_UNLESS(logger, find_class(class_info));
        if (!klass->initialized_and_no_error) {
            functions::initialize();
            functions::Class_Init(klass);
        }
        for (auto method : std::span{klass->methods, klass->methods + klass->method_count}) {
            if (method->slot == by_slot->slot) {
                return method;
            }
        }
        logger.error("Could not find method with slot {} in class {}!", by_slot->slot, class_standard_name(klass));
        log_methods(logger, klass, true);
        return nullptr;
    }
    if (auto by_vtable = std::get_if<find_method_info::by_vtable>(&info.data)) {
        auto klass = RET_DEF_UNLESS(logger, find_class(class_info));
        functions::initialize();
        if (!klass->initialized_and_no_error) {
            functions::Class_Init(klass);
        }
        auto declaring = find_class(by_vtable->declaring_class);
        if (!declaring->initialized_and_no_error) {
            functions::Class_Init(declaring);
        }
        int slot = by_vtable->slot;
        if (!functions::class_is_interface(declaring)) {
            RET_DEF_UNLESS(logger, slot < klass->vtable_count);
            auto method = klass->vtable[slot].method;

            if (method->slot != slot) {
                logger.warn("Resolving vtable slot led to a method info with a different slot! is this method abstract?");
                logger.warn("Looking for: {}, resolved to: {}", slot, method->slot);
                method = find_method(klass, slot);
                logger.info("After resolving method with slot: found method {}", fmt::ptr(method));
            }
            // resolved method slot should be the slot we asked for if it came from a non-interface
            RET_DEF_UNLESS(logger, method && slot == method->slot);

            return method;
        }
        // If the declaring class is an interface, vtable_count means nothing and instead method_count should be used.
        // Their vtables are just as valid though!
        if (slot >= declaring->method_count) {  // we tried looking for a slot that is outside the bounds of the interface vtable
            logger.error("Declaring class has a vtable that's too small, dumping find info:");
            logger.error("Instance class:                 {}", class_standard_name(klass));
            logger.error("Instance class vtable slots:    {}", klass->vtable_count);
            logger.error("Declaring class:                {}", class_standard_name(declaring));
            logger.error("Declaring class vtable slots:   {}", declaring->method_count);
            logger.error("Attempted slot:                 {}", slot);
            return nullptr;
        }
        for (decltype(klass->interface_offsets_count) i = 0; i < klass->interface_offsets_count; i++) {
            if (klass->interfaceOffsets[i].interfaceType == declaring) {
                auto offset = klass->interfaceOffsets[i].offset;
                RET_DEF_UNLESS(logger, offset + slot < klass->vtable_count);
                return klass->vtable[offset + slot].method;
            }
        }
        // if klass is an interface itself, and we haven't found the method yet, we should look in klass->methods anyway
        if (functions::class_is_interface(klass)) {
            RET_DEF_UNLESS(logger, slot < klass->method_count);
            return klass->methods[slot];
        }
        logger.error(
            "Could not find method with slot {} of interface {} in class {}!", slot, class_standard_name(declaring), class_standard_name(klass)
        );
        log_methods(logger, declaring, false);
        log_methods(logger, klass, false);
        return nullptr;
    }
    if (auto method = std::get_if<MethodInfo const*>(&info.data)) {
        return *method;
    }
    return nullptr;
}

static tuple_map<std::tuple<Il2CppClass const*, std::string>, PropertyInfo const*> properties_cache;
std::shared_mutex props_cache_lock;

PropertyInfo const* i2c::find_property(find_class_info const& class_info, find_property_info const& info) noexcept {
    if (auto by_name = std::get_if<find_property_info::by_name>(&info.data)) {
        auto klass = RET_DEF_UNLESS(logger, find_class(class_info));
        functions::initialize();

        std::shared_lock shared_lock(props_cache_lock);
        auto itr = properties_cache.find(std::make_tuple(klass, by_name->name));
        if (itr != properties_cache.end()) {
            return itr->second;
        }
        shared_lock.unlock();

        PropertyInfo const* found = nullptr;
        auto current_class = klass;
        while (!found && current_class) {
            found = functions::class_get_property_from_name(klass, by_name->name.data());
            current_class = current_class->parent;
        }
        if (!found) {
            logger.error("Could not find property {} in class {}!", by_name->name, class_standard_name(klass));
            log_properties(logger, klass, true);
        }

        std::unique_lock unique_lock(props_cache_lock);
        properties_cache[{klass, std::string(by_name->name)}] = found;
        return found;
    }
    if (auto prop = std::get_if<PropertyInfo const*>(&info.data)) {
        return *prop;
    }
    return nullptr;
}

static tuple_map<std::tuple<Il2CppClass const*, std::string>, FieldInfo*> fields_cache;
std::shared_mutex fields_cache_lock;

FieldInfo* i2c::find_field(find_class_info const& class_info, find_field_info const& info) noexcept {
    if (auto by_name = std::get_if<find_field_info::by_name>(&info.data)) {
        auto klass = RET_DEF_UNLESS(logger, find_class(class_info));
        functions::initialize();

        std::shared_lock shared_lock(fields_cache_lock);
        auto itr = fields_cache.find(std::make_tuple(klass, by_name->name));
        if (itr != fields_cache.end()) {
            return itr->second;
        }
        shared_lock.unlock();

        FieldInfo* found = nullptr;
        auto current_class = klass;
        while (!found && current_class) {
            found = functions::class_get_field_from_name(klass, by_name->name.data());
            current_class = current_class->parent;
        }
        if (!found) {
            logger.error("Could not find field {} in class {}!", by_name->name, class_standard_name(klass));
            log_fields(logger, klass, true);
        }

        std::unique_lock unique_lock(fields_cache_lock);
        fields_cache[{klass, std::string(by_name->name)}] = found;
        return found;
    }
    if (auto field = std::get_if<FieldInfo*>(&info.data)) {
        return *field;
    }
    return nullptr;
}
