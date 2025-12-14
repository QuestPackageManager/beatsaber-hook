#include "../../shared/utils/typedefs.h"
#include "../../shared/utils/il2cpp-utils-methods.hpp"
#include "../../shared/utils/hashing.hpp"
#include "utils/il2cpp-functions.hpp"
#include "utils/il2cpp-utils-classes.hpp"
#include "utils/il2cpp-utils-methods.hpp"
#include "utils/logging.hpp"
#include "utils/utils.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <shared_mutex>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace std {
    // From https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
    template<class T> void hash_combine(size_t& seed, T v) {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // Let a "sequence" type be any type that supports .size() and iteration and whose elements are hashable and support !=.
    // Calculates a hash for a sequence.
    template<class T> std::size_t hash_seq(T const& seq) noexcept {
        std::size_t seed = seq.size();
        for(auto i: seq) {
            hash_combine(seed, i);
        }
        return seed;
    }

    // Specializes std::hash for std::vector
    template<class T> struct hash<std::vector<T>> {
        std::size_t operator()(std::vector<T> const& seq) const noexcept {
            return hash_seq(seq);
        }
    };
    template <class T>
    struct hash<std::span<T>> {
        std::size_t operator()(std::span<T> const& seq) const noexcept {
            return hash_seq(seq);
        }
    };
    // Specializes std::hash for std::vector
    template <>
    struct hash<il2cpp_utils::FindMethodInfo> {
        std::size_t operator()(il2cpp_utils::FindMethodInfo const& info) const noexcept {
            auto hashPtr = std::hash<void*>{};
            auto hashSize = std::hash<std::size_t>{};

            auto hashStr = std::hash<std::string_view>{};

            return hashPtr(info.klass) ^  // hashPtr(info.returnType) ^
                   hashStr(info.name) ^ hashSize(info.argTypes.size()) ^ hashSize(info.genTypes.size());
        }
    };
}


namespace il2cpp_utils {
    typedef std::pair<std::string, std::vector<const Il2CppType*>> classesNamesTypesInnerPairType;
    static std::unordered_map<std::pair<const Il2CppClass*, std::pair<std::string, decltype(MethodInfo::parameters_count)>>, const MethodInfo*, hash_pair_3> classesNamesToMethodsCache;
    static std::unordered_map<FindMethodInfo, const MethodInfo*> classesNamesTypesToMethodsCache;
    std::mutex classNamesMethodsLock;
    std::shared_mutex classTypesMethodsLock;



#if __has_feature(cxx_exceptions)
    const MethodInfo* MakeGenericMethod(const MethodInfo* info, std::span<const Il2CppClass* const> const types)
    #else
    const MethodInfo* MakeGenericMethod(const MethodInfo* info, std::span<const Il2CppClass* const> const types) noexcept
    #endif
    {
        static auto logger = il2cpp_utils::Logger;
        il2cpp_functions::Init();
        // Ensure it exists and is generic
        THROW_OR_RET_NULL(logger, info);
        THROW_OR_RET_NULL(logger, il2cpp_functions::method_is_generic(info));
        static auto* typeClass = THROW_OR_RET_NULL(logger, il2cpp_functions::defaults->systemtype_class);
        // Create the Il2CppReflectionMethod* from the MethodInfo* using the MethodInfo's type
        auto* infoObj = il2cpp_functions::method_get_object(info, nullptr);
        if (!infoObj) {
            logger.error("[MakeGenericMethod] Failed to get MonoMethod from MethodInfo: {}", fmt::ptr(info));
            THROW_OR_RET_NULL(logger, infoObj);
        }
        // Populate generic parameters into array
        auto* arr = reinterpret_cast<Array<Il2CppReflectionType*>*>(il2cpp_functions::array_new(typeClass, types.size()));
        if (!arr) {
            logger.error("Failed to create array of length: {}", types.size());
            THROW_OR_RET_NULL(logger, arr);
        }
        int i = 0;
        for (auto* klass : types) {
            auto* typeObj = GetSystemType(klass);
            if (!typeObj) {
                logger.error("Failed to get type object from class: {}", il2cpp_functions::class_get_name_const(klass));
                THROW_OR_RET_NULL(logger, typeObj);
            }
            arr->_values[i] = typeObj;
            i++;
        }
        // Call instance function on infoObj to MakeGeneric
        // Does not need to perform type checking, since if this does not match, it will fail more miserably.
        static auto const* MakeGenericMethodInfo = ::il2cpp_utils::FindMethod(infoObj,
                                                                              "MakeGenericMethod",
                                                                              std::array<const Il2CppType*, 1>{ il2cpp_utils::ExtractType(arr) }
                                                                              );
        auto res = il2cpp_utils::RunMethodOpt<Il2CppReflectionMethod*, false>(infoObj, MakeGenericMethodInfo, arr);
        const auto* returnedInfoObj = RET_0_UNLESS(logger, res);
        if (!returnedInfoObj) {
            logger.error("Failed to get Il2CppReflectionMethod from MakeGenericMethod!");
            THROW_OR_RET_NULL(logger, returnedInfoObj);
        }
        // Get MethodInfo* back from generic instantiated method
        const auto* inflatedInfo = il2cpp_functions::method_get_from_reflection(returnedInfoObj);
        if (!inflatedInfo) {
            logger.error("Got null MethodInfo* from Il2CppReflectionMethod: {}", fmt::ptr(returnedInfoObj));
            THROW_OR_RET_NULL(logger, inflatedInfo);
        }
        // Return method to be invoked by caller
        return inflatedInfo;
    }

    const MethodInfo* ResolveMethodWithSlot(Il2CppClass* klass, uint16_t slot) noexcept {
        il2cpp_functions::Init();
        if (!klass->initialized_and_no_error) il2cpp_functions::Class_Init(klass);

        auto mend = klass->methods + klass->method_count;
        for (auto itr = klass->methods; itr != mend; itr++) {
            if ((*itr)->slot == slot) {
                return *itr;
            }
        }

        return nullptr;
    }

    const MethodInfo* ResolveVtableSlot(Il2CppClass* klass, Il2CppClass* declaringClass, uint16_t slot) noexcept {
        il2cpp_functions::Init();
        if (!klass->initialized_and_no_error) il2cpp_functions::Class_Init(klass);
        if (!declaringClass->initialized_and_no_error) il2cpp_functions::Class_Init(declaringClass);

        auto const& logger = il2cpp_utils::Logger;
        if(il2cpp_functions::class_is_interface(declaringClass)) {
            // if the declaring class is an interface,
            // vtable_count means nothing and instead method_count should be used
            // their vtables are just as valid though!
            if (slot >= declaringClass->method_count) { // we tried looking for a slot that is outside the bounds of the interface vtable
                // dump some info so the user can know which method was attempted to be resolved
                logger.error("Declaring class has a vtable that's too small, dumping resolve info:");
                logger.error("Instance class:                   {}::{}", klass->namespaze, klass->name);
                logger.error("Instance class vtable slots:      {}", klass->vtable_count);
                logger.error("Declaring class:                  {}::{}", declaringClass->namespaze, declaringClass->name);
                logger.error("Declaring class vtable slots:     {}", declaringClass->vtable_count);
                logger.error("Attempted slot:                   {}", slot);
                return {};
            }

            for (uint16_t i = 0; i < klass->interface_offsets_count; i++) {
                if(klass->interfaceOffsets[i].interfaceType == declaringClass) {
                    int32_t offset = klass->interfaceOffsets[i].offset;
                    RET_DEFAULT_UNLESS(logger, offset + slot < klass->vtable_count);
                    return klass->vtable[offset + slot].method;
                }
            }

            // if klass is an interface itself, and we haven't found the method yet, we should look in klass->methods anyway
            if (il2cpp_functions::class_is_interface(klass)) {
                RET_DEFAULT_UNLESS(logger, slot < klass->method_count);
                return klass->methods[slot];
            }

            logger.error("could not find method in slot {} of interface '{}' in class '{}'!", slot, ClassStandardName(declaringClass).c_str(), ClassStandardName(klass).c_str());
        } else {
            RET_DEFAULT_UNLESS(logger, slot < klass->vtable_count);
            auto method = klass->vtable[slot].method;

            if (method->slot != slot) {
                logger.warn("Resolving vtable slot led to a method info with a different slot! is this method abstract?");
                logger.warn("Looking for: {}, resolved to: {}", slot, method->slot);

                method = ResolveMethodWithSlot(klass, slot);
                logger.info("After resolving method with slot: found method {}", fmt::ptr(method));

            }

            // resolved method slot should be the slot we asked for if it came from a non-interface
            RET_DEFAULT_UNLESS(logger, method && slot == method->slot);

            return method;
        }
        return nullptr;
    }

    const MethodInfo* ResolveVtableSlot(Il2CppClass* klass, std::string_view declaringNamespace, std::string_view declaringClassName, uint16_t slot) noexcept {
        return ResolveVtableSlot(klass, GetClassFromName(declaringNamespace, declaringClassName), slot);
    }

    #if __has_feature(cxx_exceptions)
    const MethodInfo* FindMethodUnsafe(const Il2CppClass* klass, std::string_view methodName, int argsCount)
    #else
    const MethodInfo* FindMethodUnsafe(const Il2CppClass* klass, std::string_view methodName, int argsCount) noexcept
    #endif
    {
        il2cpp_functions::Init();
        auto const& logger = il2cpp_utils::Logger;
        RET_DEFAULT_UNLESS(logger, klass);

        // Check Cache
        auto innerPair = std::pair<std::string, decltype(MethodInfo::parameters_count)>(methodName, argsCount);
        auto key = std::pair<const Il2CppClass*, decltype(innerPair)>(klass, innerPair);
        classNamesMethodsLock.lock();
        auto itr = classesNamesToMethodsCache.find(key);
        if (itr != classesNamesToMethodsCache.end()) {
            classNamesMethodsLock.unlock();
            return itr->second;
        }
        classNamesMethodsLock.unlock();
        // Recurses through klass's parents
        auto methodInfo = il2cpp_functions::class_get_method_from_name(klass, methodName.data(), argsCount);
        if (!methodInfo) {
            logger.error("could not find method {} with {} parameters in class '{}'!", methodName.data(), argsCount, ClassStandardName(klass).c_str());
            LogMethods(logger, const_cast<Il2CppClass*>(klass), true);
            RET_DEFAULT_UNLESS(logger, methodInfo);
        }
        classNamesMethodsLock.lock();
        classesNamesToMethodsCache.emplace(key, methodInfo);
        classNamesMethodsLock.unlock();
        return methodInfo;
    }

    #if __has_feature(cxx_exceptions)
    const MethodInfo* FindMethodUnsafe(std::string_view nameSpace, std::string_view className, std::string_view methodName, int argsCount)
    #else
    const MethodInfo* FindMethodUnsafe(std::string_view nameSpace, std::string_view className, std::string_view methodName, int argsCount) noexcept
    #endif
    {
        return FindMethodUnsafe(GetClassFromName(nameSpace, className), methodName, argsCount);
    }

    #if __has_feature(cxx_exceptions)
    const MethodInfo* FindMethodUnsafe(Il2CppObject* instance, std::string_view methodName, int argsCount)
    #else
    const MethodInfo* FindMethodUnsafe(Il2CppObject* instance, std::string_view methodName, int argsCount) noexcept
    #endif
    {
        auto const& logger = il2cpp_utils::Logger;
        il2cpp_functions::Init();
        auto klass = RET_DEFAULT_UNLESS(logger, il2cpp_functions::object_get_class(instance));
        return FindMethodUnsafe(klass, methodName, argsCount);
    }

    // get class from type and match Var/MVar
    // Il2CppClass* getTypeClass(const Il2CppType* t, Il2CppGenericClass* genClass, MethodInfo const* methodInfo) {
    Il2CppClass* getTypeClass(const Il2CppType* t, MethodInfo const* methodInfo) {
        if (t->type == IL2CPP_TYPE_CLASS || t->type == IL2CPP_TYPE_VALUETYPE) {
            return il2cpp_functions::MetadataCache_GetTypeInfoFromHandle(t->data.typeHandle);
        }
        if (t->type == IL2CPP_TYPE_VAR || t->type == IL2CPP_TYPE_MVAR) {
            auto genParam = il2cpp_functions::MetadataCache_GetGenericParameterIndexFromParameter(t->data.genericParameterHandle);
            const Il2CppGenericContainer* genContainer;
            const Il2CppGenericInst* genericInst;

            // if (t->type == IL2CPP_TYPE_VAR) {
            //     auto genContainer = reinterpret_cast<const Il2CppGenericContainer*>(genClass->cached_class->genericContainerHandle);
            //     auto genParamIndex = genParam - genContainer->genericParameterStart;
            //     auto genType = genClass->context.class_inst->type_argv[genParamIndex];

            //     return getTypeClass(genType, genClass, methodInfo);
            // }
            // if (t->type == IL2CPP_TYPE_MVAR) {
            //     auto methodGenContainer = il2cpp_utils::GetGenericContainer(methodInfo);
            //     auto genParamIndex = genParam - methodGenContainer->genericParameterStart;
            //     methodInfo->genericMethod->context
            //     auto genType = methodGenContainer.->context.class_inst->type_argv[genParamIndex];

            //     return getTypeClass(genType, genClass, methodInfo);
            // }
            if (t->type == IL2CPP_TYPE_VAR) {
                // idk which is more correct, yolo
                // genContainer = reinterpret_cast<const Il2CppGenericContainer*>(methodInfo->klass->genericContainerHandle);
                genContainer = il2cpp_utils::GetGenericContainer(methodInfo);
                genericInst = methodInfo->genericMethod->context.method_inst;
            }
            if (t->type == IL2CPP_TYPE_MVAR) {
                genContainer = il2cpp_utils::GetGenericContainer(methodInfo);
                genericInst = methodInfo->genericMethod->context.method_inst;
            }

            auto genParamIndex = genParam - genContainer->genericParameterStart;
            auto genType = genericInst->type_argv[genParamIndex];

            return getTypeClass(genType, methodInfo);
        }
        if (t->type == IL2CPP_TYPE_GENERICINST) {
            auto genData = t->data.generic_class;
            return getTypeClass(genData->type, methodInfo);
        }

        return nullptr;
    }

    std::int64_t calculateWeightOfParam(Il2CppClass* methodParamClass, Il2CppClass* expectedParamClass) {
        if (!methodParamClass || !expectedParamClass) {
            return 1;
        }

        std::span<Il2CppClass const* const> methodInterfaces = { methodParamClass->implementedInterfaces, methodParamClass->interfaces_count };

        std::int64_t distance = 0;

        if (methodParamClass == expectedParamClass) {
            return distance;
        }

        bool isMethodInterface = methodParamClass->flags & TYPE_ATTRIBUTE_INTERFACE;
        bool isExpectedInterface = expectedParamClass->flags & TYPE_ATTRIBUTE_INTERFACE;

        // method is an interface, we expect a concrete type
        // avoid
        if (isExpectedInterface && !isMethodInterface) {
            return 1000;
        }

        if (isMethodInterface) {
            // if interface, just add lots of weight
            // so we choose a concrete type instead
            distance += 5;
        }

        std::span<Il2CppClass const* const> expectedInterfaces = { expectedParamClass->implementedInterfaces, expectedParamClass->interfaces_count };

        // find all interfaces which intersect with our expected type
        std::vector<Il2CppClass const*> interfaceIntersections;
        interfaceIntersections.reserve(expectedInterfaces.size());
        std::set_intersection(expectedInterfaces.begin(), expectedInterfaces.end(), methodInterfaces.begin(), methodInterfaces.end(), std::back_inserter(interfaceIntersections));
        std::size_t interfaceSharing = interfaceIntersections.size();

        while (expectedParamClass && expectedParamClass != methodParamClass) {
            if (!il2cpp_functions::class_is_assignable_from(methodParamClass, expectedParamClass)) {
                break;
            }

            expectedParamClass = expectedParamClass->parent;
            distance++;
        }

        // subtract distance by specifity of interface
        // since it allows specifity
        distance -= interfaceSharing;


        return distance;
    }

#if __has_feature(cxx_exceptions)
    const MethodInfo* FindMethod(FindMethodInfo&& info)
#else
    const MethodInfo* FindMethod(FindMethodInfo&& info) noexcept
#endif
    {
        auto logger = il2cpp_utils::Logger;
        il2cpp_functions::Init();
        auto* klass = info.klass;
        RET_DEFAULT_UNLESS(logger, klass);

        // Check Cache
        {
            std::shared_lock lock(classTypesMethodsLock);
            auto itr = classesNamesTypesToMethodsCache.find(info);
            if (itr != classesNamesTypesToMethodsCache.end()) {
                return itr->second;
            }
        }



        // Ok we look through all the methods that have the following:
        // - matches name
        // - parameters match as defined in ::il2cpp_utils::ParametersMatch
        // 
        // The resolution order goes as follows:
        // if a method parameter matches perfectly, as in the parameter types are identical, then we use that.
        // If only a single method is found, we use that.
        //
        // If multiple methods are found, the following occurs:
        // We attempt to weigh the parameters based on what we expect.
        // Then we find the lowest weighted method and use that
        // if no methods are found, we look at our parents' methods and check if those match recursively

        // Finally we add to cache and finish

        std::vector<const MethodInfo*> matches;
        matches.reserve(1);

        const MethodInfo* target = nullptr;

        auto addMethodsToMatches = [&](Il2CppClass* targetKlass) {
            // initialize klass if needed
            if (!targetKlass->initialized_and_no_error) {
                il2cpp_functions::Class_Init(targetKlass);
            }
            // Does NOT automatically recurse through klass's parents
            auto const methodsSpan = std::span(targetKlass->methods, targetKlass->method_count);
            for (auto const& current : methodsSpan) {
                if (info.name != current->name) {
                    // logger.debug("Method name does not match for method %s", current->name);
                    continue;
                }

                // strict equal
                bool isPerfect;
                if (!ParameterMatch(current, std::span<const Il2CppClass* const>(info.genTypes), std::span<const Il2CppType* const>(info.argTypes), &isPerfect)) {
                    // logger.debug("Parameters do not match for method %s", current->name);
                    continue;
                }

                // if true, perfect match
                if (isPerfect) {
                    target = current;
                    break;
                }

                auto methodParams = std::span(current->parameters, current->parameters_count);

                matches.push_back(current);
            }
        };

        // now look for matches recursively
        auto targetKlass = info.klass;
        // if we reached no parent or perfect target is found, we're done
        while (targetKlass != nullptr && target == nullptr) {
            addMethodsToMatches(targetKlass);
            targetKlass = targetKlass->parent;
        }

        // Method overload resolution
        if (!target && !matches.empty()) {
            // Only one method found, use it
            if (matches.size() == 1) {
                target = matches.front();
            }

            // multiple methods
            // time to method overload resolution
            if (!target) {
                std::vector<std::pair<const MethodInfo*, std::int64_t>> weightMap;
                weightMap.reserve(matches.size());

                // iterate methods
                for (auto const& method : matches) {
                    // overload resolution
                    std::int64_t weight = 0;

                    // weigh the methods based on their distance to the expected
                    // parameters
                    for (std::size_t i = 0; i < info.argTypes.size(); i++) {
                        auto const& expectedParamType = info.argTypes[i];
                        auto const& methodParamType = method->parameters[i];

                        auto const methodParamClass = getTypeClass(methodParamType, method);
                        auto const expectedParamClass = getTypeClass(expectedParamType, method);

                        weight += calculateWeightOfParam(methodParamClass, expectedParamClass);
                    }

                    weightMap.emplace_back(method, weight);
                }

                // if no perfect match, look for lowest weighted
                if (!target) {
                    logger.warn("[FindMethod] Found multiple methods that match for {}.{}", ClassStandardName(klass).c_str(), info.name.data());
                    for (auto const& [method, weight] : weightMap) {
                        logger.warn("[FindMethod] Weight {} Method {}", weight, method->name);
                        Paper::LoggerContext c = Paper::ConstLoggerContext("");
                        LogMethod(logger, method);
                    }

                    target = std::min_element(weightMap.begin(), weightMap.end(), [](auto a, auto b) { return a.second < b.second; })->first;
                    logger.warn("Using the following:");
                    CRASH_UNLESS(target);
                    LogMethod(logger, target);
                }
            }
        }

        if (!target) {
            std::stringstream ss;
            ss << ((matches.size() > 1) ? "found multiple matches for" : "could not find");
            ss << " method " << info.name;
            bool first = true;
            // start listing generic types
            if (!info.genTypes.empty()) {
                ss << "<";
                for (auto t : info.genTypes) {
                    if (!first) ss << ", ";
                    first = false;
                    ss << ClassStandardName(t);
                }
                ss << ">";
            }

            // start listing params
            first = true;
            ss << "(";
            for (auto t : info.argTypes) {
                if (!first) ss << ", ";
                first = false;
                ss << TypeGetSimpleName(t);
            }
            ss << ") in class '" << ClassStandardName(klass) << "'!";
            logger.error("{}", ss.str().c_str());
            LogMethods(logger, klass);
        }

        // add to cache
        {
            std::unique_lock lock(classTypesMethodsLock);
            classesNamesTypesToMethodsCache.emplace(std::move(info), target);
        }

        return target;
    }

    void LogMethods(Paper::LoggerContext const& logger, Il2CppClass const* klass, bool logParents) {
        RET_V_UNLESS(logger, klass);

        if (klass->name) {
            il2cpp_functions::Init();
            il2cpp_functions::Class_Init(const_cast<Il2CppClass*>(klass));
        }
        if (klass->method_count && !(klass->methods)) {
            logger.warn("Class is valid and claims to have methods but ->methods is null! class name: {}", ClassStandardName(klass).c_str());
            return;
        }
        if (logParents) logger.info("class name: {}", ClassStandardName(klass));

        logger.debug("method_count: {}", klass->method_count);
        for (int i = 0; i < klass->method_count; i++) {
            if (klass->methods[i]) {
                logger.debug("Method {}:", i);
                LogMethod(logger, klass->methods[i]);
            } else {
                logger.warn("Method: {} Does not exist!", i);
            }
        }
        usleep(100);  // 0.0001s
        if (logParents && klass->parent && (klass->parent != klass)) {
            LogMethods(logger, klass->parent, logParents);
        }
    }

    void LogMethod(Paper::LoggerContext const& logger, const MethodInfo* method) {
        il2cpp_functions::Init();
        RET_V_UNLESS(logger, method);

        auto flags = il2cpp_functions::method_get_flags(method, nullptr);
        std::stringstream flagStream;
        if (flags & METHOD_ATTRIBUTE_STATIC) flagStream << "static ";
        if (flags & METHOD_ATTRIBUTE_VIRTUAL) flagStream << "virtual ";
        if (flags & METHOD_ATTRIBUTE_ABSTRACT) flagStream << "abstract ";
        const auto& flagStrRef = flagStream.str();
        const char* flagStr = flagStrRef.c_str();
        auto* retType = il2cpp_functions::method_get_return_type(method);
        auto retTypeStr = TypeGetSimpleName(retType);
        auto methodName = il2cpp_functions::method_get_name(method);
        methodName = methodName ? methodName : "__noname__";
        std::stringstream paramStream;
        for (size_t i = 0; i < il2cpp_functions::method_get_param_count(method); i++) {
            if (i > 0) paramStream << ", ";
            auto* argType = il2cpp_functions::method_get_param(method, i);
            if (il2cpp_functions::type_is_byref(argType)) {
                paramStream << "out/ref ";
            }
            paramStream << TypeGetSimpleName(argType) << " ";
            auto name = il2cpp_functions::method_get_param_name(method, i);
            paramStream << (name ? name : "__noname__");
        }
        const auto& paramStrRef = paramStream.str();
        const char* paramStr = paramStrRef.c_str();
        // TODO: add <T> after methodName
        logger.debug("{}{} {}({});", flagStr, retTypeStr, methodName, paramStr);
    }

    bool IsConvertibleFrom(const Il2CppType* to, const Il2CppType* from, bool asArgs) {
        auto const& logger = il2cpp_utils::Logger;
        RET_0_UNLESS(logger, to);
        RET_0_UNLESS(logger, from);
        if (asArgs) {
            if (to->byref) {
                if (!from->byref) {
                    logger.debug("to ({}, {}) is ref/out while from ({}, {}) is not. Not convertible.",
                        TypeGetSimpleName(to), fmt::ptr(to), TypeGetSimpleName(from), fmt::ptr(from));
                    return false;
                }
            }
        }
        il2cpp_functions::Init();
        auto classTo = il2cpp_functions::class_from_il2cpp_type(to);
        auto classFrom = il2cpp_functions::class_from_il2cpp_type(from);
        bool ret = (to->type == IL2CPP_TYPE_MVAR) || il2cpp_functions::class_is_assignable_from(classTo, classFrom);
        if (!ret) {
            if (il2cpp_functions::class_is_enum(classTo)) {
                ret = IsConvertibleFrom(il2cpp_functions::class_enum_basetype(classTo), from, asArgs);
            }
        }
        return ret;
    }
}
