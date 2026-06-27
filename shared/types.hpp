#pragma once

#include "api.hpp"
#include "config.hpp"
#include "exceptions.hpp"
#include "utils.hpp"

#ifdef HAS_CODEGEN
namespace System {
    class Type;
}
#endif

namespace i2c {
    // A simple wrapper for get_system_type and cs_type_of for easier use with cordl
    struct cs_type_wrapper {
        constexpr inline cs_type_wrapper(void* t) noexcept : val(t) {}
        constexpr inline void* convert() const noexcept { return const_cast<void*>(val); }

        constexpr cs_type_wrapper(Il2CppReflectionType* t) : val(t) {}
        constexpr operator Il2CppReflectionType*() const noexcept { return static_cast<Il2CppReflectionType*>(convert()); }

#ifdef HAS_CODEGEN
        constexpr cs_type_wrapper(System::Type* t) : val(t) {}
        constexpr operator System::Type*() const noexcept { return static_cast<System::Type*>(convert()); }
#endif

        void* val;
    };

    // Returns the first matching class from the given namespace and type_name by searching through all assemblies that are loaded. (Cached)
    Il2CppClass* get_class_from_name(std::string_view namespaze, std::string_view type_name) noexcept;

    // Gets the System.Type Il2CppObject* (actually an Il2CppReflectionType*) for an Il2CppClass*
    Il2CppReflectionType* get_system_type(Il2CppClass const* klass) noexcept;
    Il2CppReflectionType* get_system_type(Il2CppType const* type) noexcept;

    // Function made by zoller27osu, modified by Sc2ad
    Il2CppClass* make_generic(Il2CppClass const* klass, i2c::view<Il2CppClass const*> args);

    // Gets the standard class name of an Il2CppClass*
    std::string class_standard_name(Il2CppClass const* klass, bool generics = true);

    // Gets a C# name of a type
    char const* type_simple_name(Il2CppType const* type);

    // Returns if a type can be converted to another, optionally ignoring byrefs if not as args
    bool is_convertible_from(Il2CppType const* to, Il2CppType const* from, bool args = false);

    enum struct match { none, convertible, exact };

    // Checks if all given parameters can be converted to the parameters of a method
    match param_match(MethodInfo const* method, i2c::view<Il2CppClass const*> gen_types, i2c::view<Il2CppType const*> arg_types);

    // Instantiates a generic MethodInfo* from the provided Il2CppClasses
    MethodInfo const* make_generic(MethodInfo const* method, i2c::view<Il2CppClass const*> types);

    /// @brief Manually creates an instance of the provided Il2CppClass*. Must be freed using gc_free_specific!
    /// The created instance's type initializer will NOT execute on another thread! Be warned!
    /// @param klass The Il2CppClass* to create an instance of.
    /// @return The created instance, or nullptr if it failed for any reason.
    Il2CppObject* create_manual(Il2CppClass const* klass);

    namespace type_markers {
        template <typename T>
        struct value_type_trait {
            static constexpr bool value = false;
        };

        template <typename T>
        struct ref_type_trait {
            static constexpr bool value = false;
        };
        // Don't specialize for just any pointer, but pointers to value types can be ref types
        template <typename T>
        requires(value_type_trait<T>::value)
        struct ref_type_trait<T*> {
            static constexpr bool value = true;
        };
    }

    namespace type_check {
        // If T is a value type - requires MARK_VAL_T or MARK_GEN_VAL_T
        template <typename T>
        concept value_type = type_markers::value_type_trait<T>::value;

        // If T is a reference type - requires MARK_REF_T, MARK_GEN_REF_T, or MARK_GEN_REF_T_PTR
        template <typename T>
        concept ref_type = type_markers::ref_type_trait<T>::value;

        template <typename T>
        concept has_get = requires { T::get(); };

        template <typename T>
        concept has_mark = value_type<T> || ref_type<T>;

        // Get the il2cpp class from only a C++ type
        template <typename T>
        struct BS_HOOK_HIDDEN no_arg_class {};

        template <typename T>
        struct BS_HOOK_HIDDEN no_arg_class<T&&> {
            static inline Il2CppClass* get() { return no_arg_class<T>::get(); }
        };

        // Get the ptr class for a value type
        template <value_type T>
        struct BS_HOOK_HIDDEN no_arg_class<T*> {
            static inline Il2CppClass* get() {
                static Il2CppClass* ptr_class = nullptr;
                if (!ptr_class) {
                    functions::initialize();
                    auto base = RET_DEF_UNLESS(i2c::logger, no_arg_class<T>::get());
                    ptr_class = functions::Class_GetPtrClass(base);
                }
                return ptr_class;
            }
        };

        // Get the il2cpp type (not class) from only a C++ type
        template <typename T>
        struct BS_HOOK_HIDDEN no_arg_type {
            static inline Il2CppType const* get() {
                auto klass = no_arg_class<T>::get();
                return klass ? &klass->byval_arg : nullptr;
            }
        };

        // Get the byref type for a reference
        template <typename T>
        struct BS_HOOK_HIDDEN no_arg_type<T&> {
            static inline Il2CppType const* get() {
                auto klass = no_arg_class<T>::get();
                return klass ? &klass->this_arg : nullptr;
            }
        };

        // A method cannot store a result back to a const ref. It is not a C# ref, so use byval
        template <typename T>
        struct BS_HOOK_HIDDEN no_arg_type<T const&> {
            static inline Il2CppType const* get() { return no_arg_type<T>::get(); }
        };

        template <typename T>
        concept has_class = has_get<no_arg_class<T>>;

        template <typename T>
        concept full_class = has_class<T> && has_mark<T>;

        template <typename T>
        concept has_type = has_get<no_arg_type<T>>;

        template <typename T>
        concept full_type = has_type<T> && has_mark<T>;

        template <typename T>
        concept wrapper_type = std::is_constructible_v<T, void*> && requires(T t) {
            { t.convert() } -> std::same_as<void*>;
        };

        template <typename T>
        concept ptr_ref_type = ref_type<T> && !wrapper_type<T>;

        template <typename T>
        concept wrapper_ref_type = ref_type<T> && wrapper_type<T>;
    }

    template <typename T>
    inline Il2CppClass* class_of() {
        return type_check::no_arg_class<T>::get();
    }

    template <typename T>
    inline Il2CppType const* type_of() {
        return type_check::no_arg_type<T>::get();
    }

    template <typename T>
    inline cs_type_wrapper cs_type_of() {
        return get_system_type(class_of<T>());
    }

    // Finds the C# type of a C++ value
    // Will use the macro-defined type for T in all cases unless it is an Il2CppObject*,
    // in which case it will find the real type of the argument at runtime
    template <typename T>
    Il2CppType const* extract_type(T const& arg) noexcept {
        if constexpr (std::is_same_v<T, Il2CppObject*>) {
            if (arg != nullptr) {
                functions::initialize();
                return functions::class_get_type(functions::object_get_class(arg));
            }
        }
        return type_of<T>();
    }

    // Converts a C++ value to a C# object or pointer
    // If Box is true, the returned value will be an Il2CppObject, otherwise a void*
    // If fake_box is also true, and the instance is a value type, it will appear boxed without actually
    // having its Il2CppObject fields set, avoiding a copy but being invalid in all but a few cases
    template <bool Box, typename T>
    auto to_object(T& class_or_inst, bool fake_box = true) noexcept {
        using R = std::conditional_t<Box, Il2CppObject*, void*>;

        void* inst;
        if constexpr (type_check::wrapper_type<T>) {
            inst = class_or_inst.convert();
        } else if constexpr (type_check::ref_type<T>) {
            inst = reinterpret_cast<void*>(class_or_inst);
        } else if constexpr (type_check::value_type<T>) {
            inst = reinterpret_cast<void*>(&class_or_inst);
        } else if constexpr (std::is_same_v<T, Il2CppClass*> || std::is_same_v<T, nullptr_t>) {
            return static_cast<R>(nullptr);
        } else {
            static_assert(false, "Invalid type passed to to_object");
        }

        if constexpr (Box && type_check::value_type<T>) {
            // Real boxing by necessity copies the struct into the boxed object, in addition to having higher overhead,
            // so modifications would have to be copied back to the original object
            if (fake_box) {
                return reinterpret_cast<R>(reinterpret_cast<char*>(inst) - sizeof(Il2CppObject));
            } else {
                functions::initialize();
                return reinterpret_cast<R>(functions::value_box(class_of<T>(), inst));
            }
        }
        return reinterpret_cast<R>(inst);
    }

    // Converts a C# object or pointer back to a C++ value
    // If Boxed is true, the instance is assumed to be an Il2CppObject*, even if T is a value type
    // This always copies the data of value types
    template <type_check::has_mark T, bool Boxed>
    auto from_object(void* inst) noexcept {
        if constexpr (Boxed && type_check::value_type<T>) {
            inst = reinterpret_cast<void*>(reinterpret_cast<char*>(inst) + sizeof(Il2CppObject));
        }
        if constexpr (type_check::wrapper_type<T>) {
            return T(inst);
        } else if constexpr (type_check::value_type<T>) {
            return *reinterpret_cast<T*>(inst);
        } else {
            return reinterpret_cast<T>(inst);
        }
    }

    /// @brief Performs an il2cpp type checked cast from T to T2.
    /// This function will throw an exception if the cast fails. i2c::result<T2> can be used to capture errors instead.
    /// @tparam T The type to cast from.
    /// @tparam T2 The type to cast to.
    /// @return A T2 of the cast value, if successful.
    template <typename T2, type_check::ref_type T>
    requires(type_check::ref_type<remove_result_t<T2>>)
    [[nodiscard]] T2 cast(T inst) noexcept(i2c::is_result_v<T2>) {
        static auto to_class = class_of<remove_result_t<T2>>();
        Il2CppObject* converted_inst = nullptr;
        if constexpr (type_check::wrapper_type<T>) {
            converted_inst = reinterpret_cast<Il2CppObject*>(inst.convert());
        } else {
            converted_inst = reinterpret_cast<Il2CppObject*>(inst);
        }
        if (!converted_inst) {
            return result_or_throw<T2>("Null pointer passed to i2c::cast!");
        }
        auto from_class = converted_inst->klass;
        if (!to_class || !from_class) {
            return result_or_throw<T2>("Invalid class in i2c::cast!");
        }
        if (from_class != to_class) {
            functions::initialize();
            if (!functions::class_is_assignable_from(to_class, from_class)) {
                return result_or_throw<T2>("The type could not be cast safely! Check your i2c::cast calls!");
            }
        }
        if constexpr (type_check::wrapper_type<remove_result_t<T2>>) {
            return remove_result_t<T2>(reinterpret_cast<void*>(converted_inst));
        } else {
            return reinterpret_cast<remove_result_t<T2>>(converted_inst);
        }
    }

    /// @brief Performs an il2cpp type checked cast from T to T2, returning nullptr if it fails.
    /// @tparam T The type to cast from.
    /// @tparam T2 The type to cast to.
    /// @return A T2 of the cast value.
    template <type_check::ref_type T2, type_check::ref_type T>
    [[nodiscard]] T2 try_cast(T inst) noexcept {
        return cast<result<T2>>(inst).value_or(nullptr);
    }

    // Allows the constexpr specification of types, for use in for example generic template parameters
    template <str_lit Namespace, str_lit Name>
    struct const_type {};

    template <str_lit Namespace, str_lit Name>
    struct BS_HOOK_HIDDEN type_check::no_arg_class<const_type<Namespace, Name>> {
        static inline Il2CppClass* get() { return get_class_from_name(Namespace.data, Name.data); }
    };
}

#define DEFINE_IL2CPP_DEFAULT_CLASS(type, field_name)              \
    template<>                                                     \
    struct BS_HOOK_HIDDEN ::i2c::type_check::no_arg_class<type> {  \
        static inline Il2CppClass* get() {                         \
            ::i2c::functions::initialize();                        \
            return ::i2c::functions::defaults->field_name##_class; \
        }                                                          \
    }
#define DEFINE_IL2CPP_DEFAULT_CLASS_VAL(type, field_name) \
    DEFINE_IL2CPP_DEFAULT_CLASS(type, field_name);        \
    MARK_VAL_T(type);
#define DEFINE_IL2CPP_DEFAULT_CLASS_REF(type, field_name) \
    DEFINE_IL2CPP_DEFAULT_CLASS(type, field_name);        \
    MARK_REF_T(type);

#define MARK_VAL_T(type) \
    template<> struct BS_HOOK_HIDDEN ::i2c::type_markers::value_type_trait<type> { static constexpr bool value = true; }
#define MARK_REF_T(type) \
    template<> struct BS_HOOK_HIDDEN ::i2c::type_markers::ref_type_trait<type> { static constexpr bool value = true; }
#define MARK_GEN_VAL_T(type) \
    template<typename... TArgs> struct BS_HOOK_HIDDEN ::i2c::type_markers::value_type_trait<type<TArgs...>> { static constexpr bool value = true; }
#define MARK_GEN_REF_T(type) \
    template<typename... TArgs> struct BS_HOOK_HIDDEN ::i2c::type_markers::ref_type_trait<type<TArgs...>> { static constexpr bool value = true; }
#define MARK_GEN_REF_T_PTR(type) \
    template<typename... TArgs> struct BS_HOOK_HIDDEN ::i2c::type_markers::ref_type_trait<type<TArgs...>*> { static constexpr bool value = true; }

#define DEFINE_IL2CPP_CLASS(type, namespaze, name)                         \
    template <>                                                            \
    struct BS_HOOK_HIDDEN ::i2c::type_check::no_arg_class<type> {          \
        static inline Il2CppClass* get() {                                 \
            static auto klass = i2c::get_class_from_name(namespaze, name); \
            return klass;                                                  \
        }                                                                  \
    }
#define DEFINE_IL2CPP_GEN_CLASS(type, namespaze, name)                               \
    template <typename... TArgs>                                                     \
    struct BS_HOOK_HIDDEN ::i2c::type_check::no_arg_class<type<TArgs...>> {          \
        static inline Il2CppClass* get() {                                           \
            static Il2CppClass* gen_inst = nullptr;                                  \
            if (!gen_inst) {                                                         \
                auto base = i2c::get_class_from_name(namespaze, name);               \
                gen_inst = i2c::make_generic(base, {no_arg_class<TArgs>::get()...}); \
            }                                                                        \
            return gen_inst;                                                         \
        }                                                                            \
    };
#define DEFINE_IL2CPP_GEN_CLASS_PTR(type, namespaze, name)                           \
    template <typename... TArgs>                                                     \
    struct BS_HOOK_HIDDEN ::i2c::type_check::no_arg_class<type<TArgs...>*> {         \
        static inline Il2CppClass* get() {                                           \
            static Il2CppClass* gen_inst = nullptr;                                  \
            if (!gen_inst) {                                                         \
                auto base = i2c::get_class_from_name(namespaze, name);               \
                gen_inst = i2c::make_generic(base, {no_arg_class<TArgs>::get()...}); \
            }                                                                        \
            return gen_inst;                                                         \
        }                                                                            \
    };

#ifdef HAS_CODEGEN
namespace System {
    class Array;
    class Delegate;
    class Object;
    class String;
    class Type;

    namespace Collections {
        class ICollection;
        class IEnumerable;
        class IList;
        class IStructuralComparable;
        class IStructuralEquatable;
    }

    class ICloneable;
}
#endif

template <typename T>
struct Array : public Il2CppArray {
    static_assert(i2c::type_check::full_type<T>, "T must be a valid C# type!");
    ALIGN_TYPE(8) T _values[IL2CPP_ZERO_LEN_ARRAY];
};

template <typename T>
struct BS_HOOK_HIDDEN ::i2c::type_check::no_arg_class<Array<T>*> {
    static inline Il2CppClass* get() {
        static Il2CppClass* klass;
        if (!klass) {
            functions::initialize();
            Il2CppClass* element_class = RET_DEF_UNLESS(::i2c::logger, no_arg_class<T>::get());
            klass = functions::array_class_get(element_class, 1);
        }
        return klass;
    }
};
MARK_GEN_REF_T_PTR(Array);

DEFINE_IL2CPP_DEFAULT_CLASS_VAL(int8_t, sbyte);
DEFINE_IL2CPP_DEFAULT_CLASS_VAL(uint8_t, byte);
DEFINE_IL2CPP_DEFAULT_CLASS_VAL(int16_t, int16);  // "short"
DEFINE_IL2CPP_DEFAULT_CLASS_VAL(uint16_t, uint16);  // "ushort"
DEFINE_IL2CPP_DEFAULT_CLASS_VAL(int32_t, int32);  // "int"
DEFINE_IL2CPP_DEFAULT_CLASS_VAL(uint32_t, uint32);  // "uint"
DEFINE_IL2CPP_DEFAULT_CLASS_VAL(int64_t, int64);  // "long"
DEFINE_IL2CPP_DEFAULT_CLASS_VAL(uint64_t, uint64);  // "ulong"

DEFINE_IL2CPP_DEFAULT_CLASS_VAL(float, single);
DEFINE_IL2CPP_DEFAULT_CLASS_VAL(double, double);

DEFINE_IL2CPP_DEFAULT_CLASS_VAL(bool, boolean);
DEFINE_IL2CPP_DEFAULT_CLASS_VAL(Il2CppChar, char);

DEFINE_IL2CPP_DEFAULT_CLASS_VAL(void, void);

DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppObject*, object);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppString*, string);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppArray*, array);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionType*, systemtype);

// From Runtime.cpp (some may need the * removed):
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppMulticastDelegate*, multicastdelegate);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppAsyncCall*, async_call);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppInternalThread*, internal_thread);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionEvent*, event_info);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppStringBuilder*, stringbuilder);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppStackFrame*, stack_frame);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionAssemblyName*, assembly_name);
#if !defined(UNITY_2021) && !defined(UNITY_6)
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionAssembly*, mono_assembly);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionField*, mono_field);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionParameter*, mono_parameter_info);
#else
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionAssembly*, assembly);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionField*, field_info);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionProperty*, property_info);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionParameter*, parameter_info);
#endif
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionModule*, module);
#if !defined(UNITY_2021) && !defined(UNITY_6)
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionPointer*, pointer);
#endif
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppSystemException*, system_exception);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppArgumentException*, argument_exception);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppMarshalByRefObject*, marshalbyrefobject);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppSafeHandle*, safe_handle);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppSortKey*, sort_key);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppErrorWrapper*, error_wrapper);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppComObject*, il2cpp_com_object);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppTypedRef, typed_reference);

DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppDelegate*, delegate);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionMonoType*, monotype);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppThread*, thread);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionRuntimeType*, runtimetype);
#if !defined(UNITY_2021) && !defined(UNITY_6)
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionMonoEventInfo*, mono_event_info);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionMethod*, mono_method);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppMethodInfo*, mono_method_info);
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppPropertyInfo*, mono_property_info);
#else
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppReflectionMethod*, method_info);
#endif
DEFINE_IL2CPP_DEFAULT_CLASS_REF(Il2CppException*, exception);

DEFINE_IL2CPP_CLASS(long double, "System", "Decimal");
MARK_VAL_T(long double);
