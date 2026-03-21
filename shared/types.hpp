#pragma once

#include "api.hpp"

namespace i2c {
    // Returns the first matching class from the given namespace and type_name by searching through all assemblies that are loaded. (Cached)
    Il2CppClass* get_class_from_name(std::string_view namespaze, std::string_view type_name);

    // Gets the System.Type Il2CppObject* (actually an Il2CppReflectionType*) for an Il2CppClass*
    Il2CppReflectionType* get_system_type(Il2CppClass const* klass);
    Il2CppReflectionType* get_system_type(Il2CppType const* type);

    // Function made by zoller27osu, modified by Sc2ad
    Il2CppClass* make_generic(Il2CppClass const* klass, std::span<Il2CppClass const* const> args);

    // Gets the standard class name of an Il2CppClass*
    std::string class_standard_name(Il2CppClass const* klass, bool generics = true);

    // Gets a C# name of a type
    char const* type_simple_name(Il2CppType const* type);

    // Returns if a type can be converted to another, optionally ignoring byrefs if not as args
    bool is_convertible_from(Il2CppType const* to, Il2CppType const* from, bool args = false);

    // Checks if all given parameters can be converted to the parameters of a method - [convertible, exact match]
    std::pair<bool, bool>
    param_match(MethodInfo const* method, std::span<Il2CppClass const* const> gen_types, std::span<Il2CppType const* const> arg_types);

    // Instantiates a generic MethodInfo* from the provided Il2CppClasses
    MethodInfo const* make_generic(MethodInfo const* method, std::span<Il2CppClass const* const> types);

    /// @brief Manually creates an instance of the provided Il2CppClass*. Must be freed using gc_free_specific!
    /// The created instance's type initializer will NOT execute on another thread! Be warned!
    /// @param klass The Il2CppClass* to create an instance of.
    /// @return The created instance, or nullptr if it failed for any reason.
    Il2CppObject* create_manual(Il2CppClass const* klass);

    namespace type_markers {
        // Used to apply the just the template of an instantiation to a type trait
        template <template <template <typename...> typename> typename Trait, typename T>
        struct decompose {
            static constexpr bool value = false;
        };
        template <template <template <typename...> typename> typename Trait, template <typename...> typename T, typename... TArgs>
        struct decompose<Trait, T<TArgs...>> {
            static constexpr bool value = Trait<T>::value;
        };

        template <typename T>
        struct value_type_trait {
            static constexpr bool value = false;
        };

        // Special generic types are needed so they can work with any specialization
        template <template <typename...> typename T>
        struct gen_value_type_trait {
            static constexpr bool value = false;
        };

        template <typename T>
        struct ref_type_trait {
            static constexpr bool value = false;
        };

        template <template <typename...> typename T>
        struct gen_ref_type_trait {
            static constexpr bool value = false;
        };
    }

    namespace type_check {
        // If T is a value type - requires MARK_VAL_T or MARK_GEN_VAL_T
        template <typename T>
        concept value_type = type_markers::value_type_trait<T>::value || type_markers::decompose<type_markers::gen_value_type_trait, T>::value;

        // If T is a reference type - requires MARK_REF_T or MARK_GEN_REF_T
        template <typename T>
        concept ref_type = type_markers::ref_type_trait<T>::value || type_markers::decompose<type_markers::gen_ref_type_trait, T>::value;

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

        // no_arg_class for generic types
        template <template <typename...> typename T>
        struct BS_HOOK_HIDDEN gen_no_arg_class {};

        // Uses gen_no_arg_class to get the class of a template instantiation
        template <template <typename...> typename T, typename... TArgs>
        requires(has_get<gen_no_arg_class<T>>)
        struct BS_HOOK_HIDDEN no_arg_class<T<TArgs...>> {
            static inline Il2CppClass* get() {
                static Il2CppClass* gen_inst = nullptr;
                if (!gen_inst) {
                    auto base = gen_no_arg_class<T>::get();
                    // std::array<Il2CppClass const*, sizeof...(TArgs)> const types{no_arg_class<TArgs>::get()...};
                    auto const types = std::array{no_arg_class<TArgs>::get()...};
                    gen_inst = i2c::make_generic(base, types);
                }
                return gen_inst;
            }
        };

        // Get the il2cpp type (not class) from only a C++ type
        template <typename T>
        struct BS_HOOK_HIDDEN no_arg_type {
            static inline Il2CppType const* get() { return &no_arg_class<T>::get()->byval_arg; }
        };

        // Get the byref type for a reference
        template <typename T>
        struct BS_HOOK_HIDDEN no_arg_type<T&> {
            static inline Il2CppType const* get() { return &no_arg_class<T>::get()->this_arg; }
        };

        // A method cannot store a result back to a const ref. It is not a C# ref, so use byval
        template <typename T>
        struct BS_HOOK_HIDDEN no_arg_type<T const&> {
            static inline Il2CppType const* get() { return no_arg_type<T>::get(); }
        };

        // template <typename T>
        // struct BS_HOOK_HIDDEN arg_type<T&> {
        //     static inline Il2CppType const* get(T& arg) {
        //         // Pure reference type is not the same as ByRef<T>. Thus, use the byval version.
        //         // Therefore, the only way to get the byref type match for any expression is to use a ByRef.
        //         // Why is this different from no_arg_type?
        //         Il2CppClass* klass = arg_class<T>::get(arg);
        //         return &klass->byval_arg;
        //     }
        // };

        template <typename T>
        concept valid_type = has_get<no_arg_class<T>> && has_mark<T>;

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
    inline Il2CppReflectionType* cs_type_of() {
        return get_system_type(class_of<T>());
    }

    /// @brief Performs an il2cpp type checked cast from T to U.
    /// Currently assumes the `klass` field is the first pointer in T (which is the case if T inherits from Il2CppObject).
    /// @tparam T The type to cast from.
    /// @tparam U The type to cast to.
    /// @return A U& of the cast value.
    template <type_check::ref_type U, type_check::ref_type T>
    [[nodiscard]] U try_cast(T inst) noexcept {
        static auto to_class = class_of<U>();
        Il2CppObject* converted_inst = nullptr;
        if constexpr (type_check::wrapper_type<T>) {
            converted_inst = reinterpret_cast<Il2CppObject*>(inst.convert());
        } else {
            converted_inst = reinterpret_cast<Il2CppObject*>(inst);
        }
        if (converted_inst) {
            functions::initialize();
            auto from_class = converted_inst->klass;
            if (!to_class || !from_class || !functions::class_is_assignable_from(to_class, from_class)) {
                converted_inst = nullptr;
            }
        }
        if constexpr (type_check::wrapper_type<U>) {
            return U(reinterpret_cast<void*>(converted_inst));
        } else {
            return reinterpret_cast<U>(converted_inst);
        }
    }
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
    template<> struct BS_HOOK_HIDDEN ::i2c::type_markers::gen_value_type_trait<type> { static constexpr bool value = true; }
// template<typename... TArgs> struct BS_HOOK_HIDDEN ::i2c::type_markers::value_type_trait<type<TArgs...>> { static constexpr bool value = true; }
#define MARK_GEN_REF_T(type) \
    template<> struct BS_HOOK_HIDDEN ::i2c::type_markers::gen_ref_type_trait<type> { static constexpr bool value = true; }

#define DEFINE_IL2CPP_CLASS(type, namespaze, name)                         \
    template <>                                                            \
    struct BS_HOOK_HIDDEN ::i2c::type_check::no_arg_class<type> {          \
        static inline Il2CppClass* get() {                                 \
            static auto klass = i2c::get_class_from_name(namespaze, name); \
            return klass;                                                  \
        }                                                                  \
    }
#define DEFINE_IL2CPP_GEN_CLASS(type, namespaze, name)                     \
    template <>                                                            \
    struct BS_HOOK_HIDDEN ::i2c::type_check::gen_no_arg_class<type> {      \
        static inline Il2CppClass* get() {                                 \
            static auto klass = i2c::get_class_from_name(namespaze, name); \
            return klass;                                                  \
        }                                                                  \
    }

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
#else
namespace System {
    typedef Il2CppArray Array;
    typedef Il2CppDelegate Delegate;
    typedef Il2CppObject Object;
    typedef Il2CppString String;
    typedef Il2CppReflectionType Type;
}
#endif

template <typename T>
struct Array : public Il2CppArray {
    static_assert(i2c::type_check::valid_type<T>, "T must be a valid C# type!");
    // static_assert(
    //     (std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_pointer_v<T> || std::is_standard_layout_v<T>) && !std::is_base_of_v<Il2CppObject,
    //     T>, "T must be a C# value type! (primitive, pointer or Struct)"
    // );
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
MARK_GEN_REF_T(Array);

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
