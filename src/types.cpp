#include "types.hpp"

#include "arrayw.hpp"
#include "debug.hpp"
#include "members.hpp"

struct hash_pair {
    template <typename T1, typename T2>
    size_t operator()(std::pair<T1, T2> const& p) const noexcept {
        return i2c::hash_combine(p.first, p.second);
    }
    using is_transparent = std::true_type;
};

// Allow lookup of {string_view, string_view} pair, instead of constructing strings
struct pair_eq {
    template <typename T1, typename T2>
    bool operator()(std::pair<T1, T1> const& lhs, std::pair<T2, T2> const& rhs) const {
        return std::get<0>(lhs) == std::get<0>(rhs) && std::get<1>(lhs) == std::get<1>(rhs);
    }
    using is_transparent = std::true_type;
};

static std::unordered_map<std::pair<std::string, std::string>, Il2CppClass*, hash_pair, pair_eq> names_to_classes;
static std::mutex name_hash_lock;

static Il2CppClass* find_nested(Il2CppClass* declaring, std::string_view type_name) {
    if (!declaring) {
        return nullptr;
    }
    auto nested_pos = type_name.find("/");
    bool deeper_nested = nested_pos != std::string::npos;

    auto sub_type_name = type_name.substr(0, nested_pos);

    void* myIter = nullptr;
    Il2CppClass* found = nullptr;
    while (Il2CppClass* nested = i2c::functions::class_get_nested_types(declaring, &myIter)) {
        if (sub_type_name == nested->name) {
            found = nested;
            break;
        }
    }

    if (!deeper_nested) {
        return found;
    }
    return find_nested(found, type_name.substr(nested_pos + 1));
}

Il2CppClass* i2c::get_class_from_name(std::string_view namespaze, std::string_view type_name) noexcept {
    functions::initialize();
    // Check cache
    name_hash_lock.lock();
    auto itr = names_to_classes.find(std::make_pair(namespaze, type_name));
    if (itr != names_to_classes.end()) {
        name_hash_lock.unlock();
        return itr->second;
    }
    name_hash_lock.unlock();

    auto nested_pos = type_name.find("/");
    bool nested = nested_pos != std::string::npos;
    if (nested) {
        // get the first part of the nested type_name
        auto declaring_name = std::string(type_name.substr(0, nested_pos));
        // get the first class, which is the declaring class
        auto declaring = get_class_from_name(namespaze, declaring_name);
        // recursively look through the nested classes of the declaring class until we run out of tokens ('/')
        // or we run into a problem where we don't find a class
        if (auto klass = find_nested(declaring, type_name.substr(nested_pos + 1))) {
            name_hash_lock.lock();
            std::pair<std::string, std::string> key(namespaze, type_name);
            names_to_classes.emplace(key, klass);
            name_hash_lock.unlock();
            return klass;
        }
    } else {
        auto dom = RET_DEF_UNLESS(logger, functions::domain_get());
        size_t assemb_count;
        Il2CppAssembly const** all_assemb = functions::domain_get_assemblies(dom, &assemb_count);
        // search through all assemblies
        for (size_t i = 0; i < assemb_count; i++) {
            auto assemb = all_assemb[i];
            auto img = functions::assembly_get_image(assemb);
            if (!img) {
                logger.error("Assembly with name: {} has a null image!", assemb->aname.name);
                continue;
            }
            if (auto klass = functions::class_from_name(img, namespaze.data(), type_name.data())) {
                name_hash_lock.lock();
                std::pair<std::string, std::string> key(namespaze, type_name);
                names_to_classes.emplace(key, klass);
                name_hash_lock.unlock();
                return klass;
            }
        }
    }

    logger.error("Could not find class with namepace: {} and name: {}", namespaze.data(), type_name.data());
    return nullptr;
}

Il2CppReflectionType* i2c::get_system_type(Il2CppClass const* klass) noexcept {
    functions::initialize();
    return get_system_type(functions::class_get_type_const(klass));
}
Il2CppReflectionType* i2c::get_system_type(Il2CppType const* type) noexcept {
    functions::initialize();
    return reinterpret_cast<Il2CppReflectionType*>(functions::type_get_object(type));
}

Il2CppClass* i2c::make_generic(Il2CppClass const* klass, i2c::view<Il2CppClass const*> args) {
    functions::initialize();
    auto class_type = RET_DEF_UNLESS(logger, get_system_type(klass));

    // Call Type.MakeGenericType on it
    ArrayW<Il2CppReflectionType*> arg_types(args.size());

    for (size_t i = 0; i < args.size(); i++) {
        if (auto arg_type = get_system_type(args[i])) {
            arg_types[i] = arg_type;
        } else {
            logger.error("Failed to get type for {}", class_standard_name(args[i]));
            return nullptr;
        }
    }

    auto reflection_type = RET_DEF_UNLESS(logger, run_method<Il2CppReflectionType*>(i2c::dynamic(class_type), "MakeGenericType", arg_types));
    auto ret = RET_DEF_UNLESS(logger, functions::class_from_system_type(reflection_type));
    return ret;
}

static void generics_to_string(Il2CppGenericClass* gen_klass, std::ostream& os) {
    auto inst = gen_klass->context.class_inst;
    if (!inst) {
        inst = gen_klass->context.method_inst;
        if (inst) {
            i2c::logger.warn("Missing class_inst in generics_to_string! Trying method_inst?");
        }
    }
    if (inst) {
        os << "<";
        for (size_t i = 0; i < inst->type_argc; i++) {
            auto typ = inst->type_argv[i];
            if (i > 0) {
                os << ", ";
            }
            os << i2c::type_simple_name(typ);
        }
        os << ">";
    } else {
        i2c::logger.warn("context->class_inst missing for gen_class!");
    }
}

std::string i2c::class_standard_name(Il2CppClass const* klass, bool generics) {
    functions::initialize();
    std::stringstream ss;
    char const* namespaze = functions::class_get_namespace(klass);
    auto declaring = functions::class_get_declaring_type(klass);
    bool has_namespace = (namespaze && namespaze[0] != '\0');
    if (!has_namespace && declaring) {
        ss << class_standard_name(declaring) << "/";
    } else {
        ss << namespaze << "::";
    }
    ss << functions::class_get_name(klass);

    if (generics) {
        functions::class_is_generic(klass);
        auto gen_class = klass->generic_class;
        if (gen_class) {
            generics_to_string(gen_class, ss);
        }
    }
    return ss.str();
}

static std::unordered_map<Il2CppClass*, char const*> type_names;
static std::mutex type_names_lock;

char const* i2c::type_simple_name(Il2CppType const* type) {
    functions::initialize();

    type_names_lock.lock();
    if (type_names.empty()) {
        auto defaults = functions::defaults;
        type_names[defaults->boolean_class] = "bool";
        type_names[defaults->byte_class] = "byte";
        type_names[defaults->sbyte_class] = "sbyte";
        type_names[defaults->char_class] = "char";
        type_names[defaults->single_class] = "float";
        type_names[defaults->double_class] = "double";
        type_names[defaults->int16_class] = "short";
        type_names[defaults->uint16_class] = "ushort";
        type_names[defaults->int32_class] = "int";
        type_names[defaults->uint32_class] = "uint";
        type_names[defaults->int64_class] = "long";
        type_names[defaults->uint64_class] = "ulong";
        type_names[defaults->object_class] = "object";
        type_names[defaults->string_class] = "string";
        type_names[defaults->void_class] = "void";
    }

    auto p = type_names.find(functions::class_from_il2cpp_type(type));
    if (p != type_names.end()) {
        char const* name = p->second;
        type_names_lock.unlock();
        return name;
    } else {
        type_names_lock.unlock();
        return functions::type_get_name(type);
    }
}

bool i2c::is_convertible_from(Il2CppType const* to, Il2CppType const* from, bool args) {
    RET_DEF_UNLESS(logger, to);
    RET_DEF_UNLESS(logger, from);
    if (args) {
        if (to->byref && !from->byref) {
            // logger.debug(
            //     "to ({}, {}) is ref/out while from ({}, {}) is not. Not convertible.",
            //     type_simple_name(to),
            //     fmt::ptr(to),
            //     type_simple_name(from),
            //     fmt::ptr(from)
            // );
            return false;
        }
    }
    functions::initialize();
    auto class_to = functions::class_from_il2cpp_type(to);
    auto class_from = functions::class_from_il2cpp_type(from);
    bool ret = (to->type == IL2CPP_TYPE_MVAR) || functions::class_is_assignable_from(class_to, class_from);
    if (!ret && functions::class_is_enum(class_to)) {
        ret = is_convertible_from(functions::class_enum_basetype(class_to), from, args);
    }
    return ret;
}

static Il2CppGenericContainer const* get_generic_container(MethodInfo const* method) {
    if (!method->is_generic) {
        SAFE_ABORT("Method is not generic!");
    }

    if (method->is_inflated) {
        auto gen_info = method->genericMethod;
#if defined(UNITY_2021) || defined(UNITY_6)
        return reinterpret_cast<Il2CppGenericContainer const*>(gen_info->methodDefinition->genericContainerHandle);
#else
        return gen_info->methodDefinition->genericContainerHandle;
#endif
    } else {
#if defined(UNITY_2021) || defined(UNITY_6)
        return reinterpret_cast<Il2CppGenericContainer const*>(method->genericContainerHandle);
#else
        return = method->genericContainer;
#endif
    }
}

i2c::match i2c::param_match(MethodInfo const* method, i2c::view<Il2CppClass const*> gen_types, i2c::view<Il2CppType const*> arg_types) {
    if (method->parameters_count != arg_types.size()) {
        // logger.warn("Potential method match had wrong number of parameters {} (expected {})", method->parameters_count, arg_types.size());
        return match::none;
    }

    Il2CppGenericContainer const* container;
    int32_t gen_count = 0;
    if (method->is_generic) {
        container = get_generic_container(method);
        gen_count = container->type_argc;
    }

    if ((size_t) gen_count != gen_types.size()) {
        // logger.warn("Potential method match had wrong number of generics {} (expected {})", gen_count, gen_types.size());
        // logger.warn("is generic {} is inflated {}", method->is_generic, method->is_inflated);
        return match::none;
    }
    bool identical = true;
    // TODO: supply boolStrictMatch and use type_equals instead of is_convertible_from if supplied?
    for (decltype(method->parameters_count) i = 0; i < method->parameters_count; i++) {
        auto param_type = method->parameters[i];
        if (arg_types[i] == nullptr) {
            logger.warn("Arg type {} is null. Method: {}", i, fmt::ptr(method));
            log_method(logger, method);
            continue;
        }
        if (param_type->type == IL2CPP_TYPE_MVAR) {
            if (gen_count == 0) {
                logger.warn("No generic args to extract paramIdx {}", i);
                continue;
            }
            functions::initialize();
            auto gen_idx = functions::MetadataCache_GetGenericParameterIndexFromParameter(param_type->data.genericParameterHandle) -
                           container->genericParameterStart;
            if (gen_idx < 0) {
                logger.warn("Extracted invalid gen_idx {} from parameter {}", gen_idx, i);
                continue;
            }
            if (gen_idx >= gen_count) {
                logger.warn(
                    "ParameterMatch was not supplied enough gen_types to determine type of parameter {} "
                    "(had {}, needed {})!",
                    i,
                    gen_count,
                    gen_idx
                );
                continue;
            }

            auto klass = gen_types[gen_idx];
            param_type = (param_type->byref) ? &klass->this_arg : &klass->byval_arg;
        }
        // parameters are identical if every param matches exactly!
        identical &= param_type == arg_types[i];

        // TODO: just because two parameter lists match doesn't necessarily mean this is the best match...
        if (!is_convertible_from(param_type, arg_types[i], true)) {
            return match::none;
        }
    }
    return identical ? match::exact : match::convertible;
}

MethodInfo const* i2c::make_generic(MethodInfo const* method, i2c::view<Il2CppClass const*> types) {
    // Ensure it exists and is generic
    THROW_UNLESS(logger, method && method->is_generic);
    // Create the Il2CppReflectionMethod* from the MethodInfo* using the MethodInfo's type
    functions::initialize();
    auto method_object = functions::method_get_object(method, nullptr);
    if (!method_object) {
        logger.error("Failed to get ReflectionMethod from MethodInfo: {}", fmt::ptr(method));
        THROW_UNLESS(logger, method_object);
    }
    // Populate generic parameters into array
    ArrayW<Il2CppReflectionType*> types_array(types.size());
    int i = 0;
    for (auto klass : types) {
        types_array[i] = get_system_type(klass);
        if (!types_array[i]) {
            logger.error("Failed to get type object from class: {}", class_standard_name(klass));
            THROW_UNLESS(logger, types_array[i]);
        }
        i++;
    }
    // Call instance function on method object to MakeGeneric
    auto inflated_object = run_method<Il2CppReflectionMethod*>(i2c::dynamic(method_object), "MakeGenericMethod", types_array);
    if (!inflated_object) {
        logger.error("Failed to run MakeGenericMethod!");
        THROW_UNLESS(logger, inflated_object);
    }
    // Get MethodInfo* back from generic instantiated method
    auto inflated = functions::method_get_from_reflection(inflated_object);
    if (!inflated) {
        logger.error("Got null MethodInfo* from Il2CppReflectionMethod: {}", fmt::ptr(inflated_object));
        THROW_UNLESS(logger, inflated);
    }
    // Return method to be invoked by caller
    return inflated;
}

Il2CppObject* i2c::create_manual(Il2CppClass const* klass) {
    if (!klass) {
        logger.error("Cannot create an object with a null class!");
        return nullptr;
    }
    if (!klass->initialized) {
        logger.error("Cannot create an object that does not have an initialized class: {}", fmt::ptr(klass));
        return nullptr;
    }
    auto obj = reinterpret_cast<Il2CppObject*>(gc_alloc_specific(klass->instance_size));
    if (!obj) {
        logger.error("Failed to allocate GC specific area for instance size: {}", klass->instance_size);
        return nullptr;
    }
    obj->klass = const_cast<Il2CppClass*>(klass);
    // Call cctor, we don't bother making a new thread for the type initializer. BE WARNED!
    if (klass->has_cctor && !klass->cctor_finished_or_no_cctor && !klass->cctor_started) {
        obj->klass->cctor_started = true;
        run_method(obj->klass, ".cctor");
        obj->klass->cctor_finished_or_no_cctor = true;
    }
    return obj;
}
