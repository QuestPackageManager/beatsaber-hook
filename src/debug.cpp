#include "debug.hpp"

#include "alphanum.hpp"
#include "binary.hpp"
#include "types.hpp"
#include "utils/HashUtils.h"
#include "utils/Il2CppHashMap.h"
#include "utils/StringUtils.h"

#include <map>

struct NamespaceAndNamePairHash {
    size_t operator()(std::pair<char const*, char const*> const& pair) const {
        return il2cpp::utils::HashUtils::Combine(il2cpp::utils::StringUtils::Hash(pair.first), il2cpp::utils::StringUtils::Hash(pair.second));
    }
};

struct NamespaceAndNamePairEquals {
    bool operator()(std::pair<char const*, char const*> const& p1, std::pair<char const*, char const*> const& p2) const {
        return !strcmp(p1.first, p2.first) && !strcmp(p1.second, p2.second);
    }
};

struct Il2CppNameToTypeHandleHashTable :
    public Il2CppHashMap<std::pair<char const*, char const*>, Il2CppMetadataTypeHandle, NamespaceAndNamePairHash, NamespaceAndNamePairEquals> {
    typedef Il2CppHashMap<std::pair<char const*, char const*>, Il2CppMetadataTypeHandle, NamespaceAndNamePairHash, NamespaceAndNamePairEquals> Base;
    Il2CppNameToTypeHandleHashTable() : Base() {}
};

typedef struct Il2CppImageGlobalMetadata {
    TypeDefinitionIndex typeStart;
    TypeDefinitionIndex exportedTypeStart;
    CustomAttributeIndex customAttributeStart;
    MethodIndex entryPointIndex;
    Il2CppImage const* image;
} Il2CppImageGlobalMetadata;

static int indent = -1;
static int max_indent = 0;
static std::unordered_set<Il2CppClass*> logged_classes;

void i2c::log_class(Paper::LoggerContext const& logger, Il2CppClass* klass, bool parents) noexcept {
    functions::initialize();
    RET_V_UNLESS(logger, klass);

    if (logged_classes.count(klass)) {
        logger.debug("Already logged {}!", fmt::ptr(klass));
        return;
    }
    logged_classes.insert(klass);

    RET_V_UNLESS(logger, klass->klass == klass);  // otherwise, klass is likely NOT an Il2CppClass*!
    RET_V_UNLESS(logger, klass->name);  // ditto

    indent++;
    // Note: il2cpp stops at GenericMetadata::MaximumRuntimeGenericDepth (which is 8)
    max_indent = std::max(indent, max_indent);

    bool methods_inited = false;
    if (klass->name) {
        // Note: unless vm/Class.cpp is wrong, Class::Init always returns true
        functions::Class_Init(klass);
        if (klass->initialized_and_no_error) {
            methods_inited = true;
        }
    }

    logger.debug("{} ======================CLASS INFO FOR CLASS: {}======================", indent, class_standard_name(klass));
    void* iter = nullptr;
    if (!methods_inited) {
// log results of Class::Init
#ifdef UNITY_6
        logger.warn(
            "klass->initialized: {}, init_pending: {}, initialized_and_no_error: {}, initializationExceptionGCHandle: {}",
            (bool) klass->initialized,
            (bool) klass->init_pending,
            (bool) klass->initialized_and_no_error,
            klass->initializationExceptionGCHandle
        );
#elif defined(UNITY_2021)
        logger.warn(
            "klass->initialized: {}, init_pending: {}, initialized_and_no_error: {}, initializationExceptionGCHandle: {:X}",
            (bool) klass->initialized,
            (bool) klass->init_pending,
            (bool) klass->initialized_and_no_error,
            klass->initializationExceptionGCHandle
        );
#else
        logger.warn(
            "klass->initialized: {}, init_pending: {}, has_initialization_error: {}, initializationExceptionGCHandle: {:X}",
            (bool) klass->initialized,
            (bool) klass->init_pending,
            (bool) klass->has_initialization_error,
            klass->initializationExceptionGCHandle
        );
#endif
        auto m1 = functions::class_get_methods(klass, &iter);  // attempt again to initialize the method data
        if (klass->method_count && !klass->methods) {
            logger.error("Class::Init and class_get_methods failed to initialize klass->methods! class_get_methods returned: {}", fmt::ptr(m1));
            if (m1) {
                log_method(logger, m1);
            }
        }
    }

    logger.debug("Pointer: {}", fmt::ptr(klass));
    logger.debug("Type Token: {}", functions::class_get_type_token(klass));
    auto type_def_idx = functions::MetadataCache_GetIndexForTypeDefinition(klass->generic_class ? klass->generic_class->cached_class : klass);
    logger.debug("TypeDefinitionIndex: {}", type_def_idx);
#if !defined(UNITY_2021) && !defined(UNITY_6)
    // Repair the typeDefinition value if it was null but we found one
    if (!klass->typeDefinition && typeDefIdx > 0) {
        klass->typeDefinition = functions::MetadataCache_GetTypeDefinitionFromIndex(typeDefIdx);
    }
    logger.debug("Type definition: {}", fmt::ptr(klass->typeDefinition));
#endif

    logger.debug("Assembly Name: {}", functions::class_get_assemblyname(klass));

    auto type = functions::class_get_type(klass);
    if (type) {
        logger.debug("Type name: {}", functions::type_get_name(type));
        if (auto refl_name = functions::Type_GetName(type, IL2CPP_TYPE_NAME_FORMAT_REFLECTION)) {
            logger.debug("Type reflection name: {}", refl_name);
            functions::free(refl_name);
        }
        logger.debug("Fully qualifed type name: {}", functions::type_get_assembly_qualified_name(type));
    }
    logger.debug("Rank: {}", functions::class_get_rank(klass));
    logger.debug("Flags: 0x{:08X}", functions::class_get_flags(klass));
    logger.debug("Event Count: {}", klass->event_count);
    logger.debug("Method Count: {}", klass->method_count);
    logger.debug("Is Generic: {}", functions::class_is_generic(klass));
    logger.debug("Is Abstract: {}", functions::class_is_abstract(klass));

// Some methods, such as GenericClass::GetClass, may not initialize all fields in Il2CppClass, and thus not meet all implicit contracts defined by the
// comments in Il2CppClass's struct definition. But unless we're blind, the only method that sets is_generic on non-methods is
// MetadataCache::FromTypeDefinition. That method also contains the only assignment of genericContainerIndex. Therefore, this code makes only the
// following assumptions:
// 1. If is_generic is set, then genericContainerIndex was also intentionally set (even if it's 0) and is not -1 (invalid)
// 2. Even if is_generic wasn't set, a positive genericContainerIndex was intentionally set that way and is a valid index.
#if defined(UNITY_2021) || defined(UNITY_6)
    auto class_gen_container_idx = functions::MetadataCache_GetGenericContainerIndex(klass);
#else
    auto class_gen_container_idx = klass->genericContainerIndex;
#endif

    if (klass->is_generic || class_gen_container_idx > 0) {
        auto gen_container = functions::MetadataCache_GetGenericContainerFromIndex(class_gen_container_idx);
        logger.debug(
            "genContainer: idx {}, ownerIndex: {}, is_method: {}", class_gen_container_idx, gen_container->ownerIndex, gen_container->is_method
        );
        if (gen_container->ownerIndex != type_def_idx) {
            logger.error("gen_container ownerIndex mismatch!");
        }
        for (int i = 0; i < gen_container->type_argc; i++) {
            auto gen_param_idx = gen_container->genericParameterStart + i;
            auto gen_param = functions::MetadataCache_GetGenericParameterFromIndex(gen_param_idx);
            if (gen_param) {
                logger.debug(
                    "gen_param #{}, idx {}: owner_idx {}, name {}, num {}, flags (see "
                    "IL2CPP_GENERIC_PARAMETER_ATTRIBUTE_X in il2cpp-tabledefs.h) 0x{:02x}",
                    i,
                    gen_param_idx,
                    gen_param->ownerIndex,
                    functions::MetadataCache_GetStringFromIndex(gen_param->nameIndex),
                    gen_param->num,
                    gen_param->flags
                );
            } else {
                logger.warn("gen_param {}, idx {}: null", i, gen_param_idx);
            }
        }
    } else {
        logger.debug("genericContainerIndex: {}", class_gen_container_idx);
    }

    logger.debug("{} =========METHODS=========", indent);
    log_methods(logger, klass);
    logger.debug("{} =======END METHODS=======", indent);

    auto declaring = functions::class_get_declaring_type(klass);
    logger.debug("declaring type: {} ({})", fmt::ptr(declaring), declaring ? class_standard_name(declaring) : "");
    if (declaring && parents) {
        log_class(logger, declaring, parents);
    }
    auto element = functions::class_get_element_class(klass);
    logger.debug("element class: {} ('{}', self = {})", fmt::ptr(element), element ? class_standard_name(element) : "", fmt::ptr(klass));
    if (element && element != klass && parents) {
        log_class(logger, element, parents);
    }

    logger.debug("{} =======PROPERTIES=======", indent);
    log_properties(logger, klass);
    logger.debug("{} =====END PROPERTIES=====", indent);
    logger.debug("{} =========FIELDS=========", indent);
    log_fields(logger, klass);
    logger.debug("{} =======END FIELDS=======", indent);

    auto parent = functions::class_get_parent(klass);
    logger.debug("parent: {} ({})", fmt::ptr(parent), parent ? class_standard_name(parent) : "");
    if (parent && parents) {
        log_class(logger, parent, parents);
    }
    logger.debug("{}, ==================================================================================", indent);
    indent--;
}

static std::unordered_map<Il2CppClass*, std::map<std::string, Il2CppGenericClass*, doj::alphanum_less<std::string>>> class_to_generic;

static std::string gen_class_standard_name(Il2CppGenericClass* gen_class) {
    if (gen_class->cached_class) {
        return i2c::class_standard_name(gen_class->cached_class);
    }
    if (i2c::functions::MetadataCache_GetIndexForTypeDefinition(gen_class->cached_class) != kTypeDefinitionIndexInvalid) {
        i2c::functions::initialize();
        auto klass = i2c::functions::GenericClass_GetClass(gen_class);
        return i2c::class_standard_name(klass);
    }
    return "?";
}

static void build_generics_map() {
    i2c::functions::initialize();
    auto metadata_reg = RET_V_UNLESS(i2c::logger, i2c::functions::s_Il2CppMetadataRegistration);
    i2c::logger.debug("metadata_reg: {}, offset = {:X}", fmt::ptr(metadata_reg), ((uintptr_t) metadata_reg) - i2c::binary::get_real_offset(0));

    int uncached_class_count = 0;
    for (int i = 0; i < metadata_reg->genericClassesCount; i++) {
        Il2CppGenericClass* gen_class = metadata_reg->genericClasses[i];
        if (!gen_class) {
            continue;
        }
        if (!gen_class->cached_class) {
            uncached_class_count++;
        }
        std::string gen_class_name = gen_class_standard_name(gen_class);

        auto type_def_class = i2c::functions::GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex(
            i2c::functions::MetadataCache_GetIndexForTypeDefinition(gen_class->cached_class)
        );
        if (type_def_class) {
            class_to_generic[type_def_class][gen_class_name] = gen_class;
        }
    }
    i2c::logger.debug(
        "uncached_class_count: {} ({} proportion of total)", uncached_class_count, uncached_class_count * 1.0 / metadata_reg->genericClassesCount
    );
}

static void
add_nested_types_to_name(Il2CppNameToTypeHandleHashTable* table, char const* namespaze, std::string const& parent_name, Il2CppClass* klass) {
    i2c::functions::initialize();
    std::string name = parent_name + "/" + klass->name;
    char* p_name = reinterpret_cast<char*>(i2c::gc_alloc_specific(name.size() + 1 * sizeof(char)));
    strlcpy(p_name, name.c_str(), name.length() + 1);

    auto type_definition = i2c::functions::MetadataCache_GetTypeDefinition(klass);
    // GlobalMetadata.cpp shows Il2CppMetadataTypeHandle == Il2CppTypeDefinition const*
    auto handle = reinterpret_cast<Il2CppMetadataTypeHandle>(type_definition);
    table->insert(std::make_pair(std::make_pair(namespaze, (char const*) p_name), handle));

    void* iter = NULL;
    while (Il2CppClass* nestedClass = i2c::functions::class_get_nested_types(klass, &iter)) {
        add_nested_types_to_name(table, namespaze, name, nestedClass);
    }
}

static void add_nested_types_to_name(Il2CppImage const* img, Il2CppTypeDefinition const* type_definition) {
    i2c::functions::initialize();
    for (int i = 0; i < type_definition->nested_type_count; ++i) {
        Il2CppClass* klass = i2c::functions::MetadataCache_GetNestedTypeFromIndex(type_definition->nestedTypesStart + i);
        add_nested_types_to_name(
            img->nameToClassHashTable,
            i2c::functions::MetadataCache_GetStringFromIndex(type_definition->namespaceIndex),
            i2c::functions::MetadataCache_GetStringFromIndex(type_definition->nameIndex),
            klass
        );
    }
}

static void add_type_to_name(Il2CppImage const* img, TypeDefinitionIndex index) {
    i2c::functions::initialize();
    Il2CppTypeDefinition const* type_definition = i2c::functions::MetadataCache_GetTypeDefinitionFromIndex(index);
    // don't add nested types
    if (type_definition->declaringTypeIndex != kTypeIndexInvalid) {
        return;
    }

    if (img != i2c::functions::get_corlib()) {
        add_nested_types_to_name(img, type_definition);
    }
    // GlobalMetadata.cpp shows Il2CppMetadataTypeHandle == Il2CppTypeDefinition const*
    auto handle = reinterpret_cast<Il2CppMetadataTypeHandle>(type_definition);
    img->nameToClassHashTable->insert(
        std::make_pair(
            std::make_pair(
                i2c::functions::MetadataCache_GetStringFromIndex(type_definition->namespaceIndex),
                i2c::functions::MetadataCache_GetStringFromIndex(type_definition->nameIndex)
            ),
            handle
        )
    );
}

void i2c::log_classes(Paper::LoggerContext const& logger, ::std::string_view prefix, bool parents) noexcept {
    functions::initialize();
    build_generics_map();

    // Begin prefix matching
    std::map<std::string, Il2CppClass*, doj::alphanum_less<std::string>> matches;
    // Get il2cpp domain
    auto dom = functions::domain_get();
    // Get all il2cpp assemblies
    size_t size;
    auto assembs = functions::domain_get_assemblies(dom, &size);

    for (size_t i = 0; i < size; ++i) {
        // Get image for each assembly
        if (!assembs[i]) {
            logger.warn("Assembly {} was null! Skipping.", i);
            continue;
        }
        logger.debug("Scanning assembly \"{}\"", assembs[i]->aname.name);
        auto img = functions::assembly_get_image(assembs[i]);
        if (!img) {
            logger.warn("Assembly's image was null! Skipping.");
            continue;
        }

        if (img->nameToClassHashTable == nullptr) {
            logger.debug("Assembly's nameToClassHashTable is empty. Populating it instead.");

            img->nameToClassHashTable = new Il2CppNameToTypeHandleHashTable();
            auto metadata = reinterpret_cast<Il2CppImageGlobalMetadata const*>(img->metadataHandle);

            for (uint32_t index = 0; index < img->typeCount; index++) {
                TypeDefinitionIndex typeIndex = metadata->typeStart + index;
                add_type_to_name(img, typeIndex);
            }

            for (uint32_t index = 0; index < img->exportedTypeCount; index++) {
                auto typeIndex = functions::MetadataCache_GetExportedTypeFromIndex(metadata->exportedTypeStart + index);
                if (typeIndex != kTypeIndexInvalid) {
                    add_type_to_name(img, typeIndex);
                }
            }
        }

        for (auto itr = img->nameToClassHashTable->begin(); itr != img->nameToClassHashTable->end(); ++itr) {
            // ->first is a KeyWrapper(pair(namespaceName, className))
            // ->second is TypeDefinitionIndex
            if (std::string_view(itr->first.key.second).starts_with(prefix)) {
                // Convert TypeDefinitionIndex --> class
                auto klass = functions::MetadataCache_GetTypeInfoFromHandle(itr->second);
                matches[class_standard_name(klass)] = klass;
            }
        }
    }

    usleep(1000);  // 0.001s
    logger.debug("log_classes:");
    for (auto const& pair : matches) {
        log_class(logger, pair.second, parents);
        indent = -1;
        for (auto const& genPair : class_to_generic[pair.second]) {
            logger.debug("{}", genPair.first.c_str());
        }
        usleep(1000);  // 0.001s
    }
    logger.debug("log_classes({}) is complete.", prefix.data());
    logger.debug("max_indent: {}", max_indent);
}

void i2c::log_method(Paper::LoggerContext const& logger, MethodInfo const* method) {
    functions::initialize();
    RET_V_UNLESS(logger, method);

    auto flags = functions::method_get_flags(method, nullptr);
    std::stringstream flagStream;
    if (flags & METHOD_ATTRIBUTE_STATIC) {
        flagStream << "static ";
    }
    if (flags & METHOD_ATTRIBUTE_VIRTUAL) {
        flagStream << "virtual ";
    }
    if (flags & METHOD_ATTRIBUTE_ABSTRACT) {
        flagStream << "abstract ";
    }
    auto const& flagStrRef = flagStream.str();
    char const* flagStr = flagStrRef.c_str();
    auto retType = functions::method_get_return_type(method);
    auto retTypeStr = type_simple_name(retType);
    auto methodName = functions::method_get_name(method);
    methodName = methodName ? methodName : "__noname__";
    std::stringstream paramStream;
    for (size_t i = 0; i < functions::method_get_param_count(method); i++) {
        if (i > 0) {
            paramStream << ", ";
        }
        auto argType = functions::method_get_param(method, i);
        if (functions::type_is_byref(argType)) {
            paramStream << "out/ref ";
        }
        paramStream << type_simple_name(argType) << " ";
        auto name = functions::method_get_param_name(method, i);
        paramStream << (name ? name : "__noname__");
    }
    auto const& paramStrRef = paramStream.str();
    char const* paramStr = paramStrRef.c_str();
    // TODO: add <T> after methodName
    logger.debug("{}{} {}({});", flagStr, retTypeStr, methodName, paramStr);
}

void i2c::log_methods(Paper::LoggerContext const& logger, Il2CppClass const* klass, bool parents) {
    RET_V_UNLESS(logger, klass);

    if (klass->name) {
        functions::initialize();
        functions::Class_Init(const_cast<Il2CppClass*>(klass));
    }
    if (klass->method_count && !(klass->methods)) {
        logger.warn("Class is valid and claims to have methods but ->methods is null! class name: {}", class_standard_name(klass).c_str());
        return;
    }
    if (parents) {
        logger.info("class name: {}", class_standard_name(klass));
    }

    logger.debug("method_count: {}", klass->method_count);
    for (int i = 0; i < klass->method_count; i++) {
        if (klass->methods[i]) {
            logger.debug("Method {}:", i);
            log_method(logger, klass->methods[i]);
        } else {
            logger.warn("Method: {} Does not exist!", i);
        }
    }
    usleep(100);  // 0.0001s
    if (parents && klass->parent && (klass->parent != klass)) {
        log_methods(logger, klass->parent, parents);
    }
}

void i2c::log_field(Paper::LoggerContext const& logger, FieldInfo* field) {
    functions::initialize();
    RET_V_UNLESS(logger, field);

    auto flags = functions::field_get_flags(field);
    char const* flagStr = (flags & FIELD_ATTRIBUTE_STATIC) ? "static " : "";
    auto type = functions::field_get_type(field);
    auto typeStr = type_simple_name(type);
    auto name = functions::field_get_name(field);
    name = name ? name : "__noname__";
    auto offset = functions::field_get_offset(field);

    logger.debug("{}{} {}; // 0x{:X}, flags: 0x{:04X}", flagStr, typeStr, name, offset, flags);
}

void i2c::log_fields(Paper::LoggerContext const& logger, Il2CppClass* klass, bool parents) {
    functions::initialize();
    RET_V_UNLESS(logger, klass);

    void* myIter = nullptr;
    FieldInfo* field;
    if (klass->name) {
        functions::Class_Init(klass);
    }
    if (parents) {
        logger.info("class name: {}", class_standard_name(klass).c_str());
    }

    logger.debug("field_count: {}", klass->field_count);
    while ((field = functions::class_get_fields(klass, &myIter))) {
        log_field(logger, field);
    }
    usleep(100);
    if (parents && klass->parent && klass->parent != klass) {
        log_fields(logger, klass->parent, parents);
    }
}

void i2c::log_property(Paper::LoggerContext const& logger, PropertyInfo const* prop) {
    functions::initialize();
    RET_V_UNLESS(logger, prop);

    auto flags = functions::property_get_flags(prop);
    char const* flagStr = (flags & FIELD_ATTRIBUTE_STATIC) ? "static " : "";
    auto name = functions::property_get_name(prop);
    name = name ? name : "__noname__";
    auto getter = functions::property_get_get_method(prop);
    auto getterName = getter ? functions::method_get_name(getter) : "";
    auto setter = functions::property_get_set_method(prop);
    auto setterName = setter ? functions::method_get_name(setter) : "";
    Il2CppType const* type = nullptr;
    if (getter) {
        type = functions::method_get_return_type(getter);
    } else if (setter) {
        type = functions::method_get_param(setter, 0);
    }
    auto typeStr = type ? type_simple_name(type) : "?type?";

    logger.debug("{}{} {} {{ {}; {}; }}; // flags: 0x{:04x}", flagStr, typeStr, name, getterName, setterName, flags);
}

void i2c::log_properties(Paper::LoggerContext const& logger, Il2CppClass* klass, bool parents) {
    functions::initialize();
    RET_V_UNLESS(logger, klass);

    void* myIter = nullptr;
    PropertyInfo const* prop;
    if (klass->name) {
        functions::Class_Init(klass);
    }
    if (parents) {
        logger.info("class name: {}", class_standard_name(klass));
    }

    logger.debug("property_count: {}", klass->property_count);
    while ((prop = functions::class_get_properties(klass, &myIter))) {
        log_property(logger, prop);
    }
    usleep(100);
    if (parents && klass->parent && klass->parent != klass) {
        log_properties(logger, klass->parent, parents);
    }
}
