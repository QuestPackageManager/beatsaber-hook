#pragma once

#include "find.hpp"

namespace i2c {
    template <typename T>
    Il2CppType const* extract_type(T const& arg) {
        if constexpr (std::is_same_v<T, Il2CppObject*>) {
            functions::initialize();
            return functions::class_get_type(functions::object_get_class(arg));
        }
        return type_of<T>();
    }

    template <bool Box, typename T>
    auto to_object(T& class_or_inst, bool fake_box = true) {
        using R = std::conditional_t<Box, Il2CppObject*, void*>;

        void* inst;
        if constexpr (type_check::wrapper_type<T>) {
            inst = class_or_inst.convert();
        } else if constexpr (type_check::ref_type<T>) {
            inst = reinterpret_cast<void*>(class_or_inst);
        } else if constexpr (type_check::value_type<T>) {
            inst = reinterpret_cast<void*>(&class_or_inst);
        } else {
            return static_cast<R>(nullptr);
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

    template <type_check::valid_type T, bool Boxed>
    auto from_object(void* inst) {
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

    // The purpose of the separate implementation and overloads is to preserve the compile time type of class_or_inst,
    // while still allowing for the elegant construction of find_class_info with multiple parameters (namepace + name)
    template <type_check::valid_type T = void, type_check::valid_type... TArgs>
    T run_method_impl(find_class_info klass, auto&& class_or_inst, find_method_info method, auto&&... args) {
        if (auto name = method.only_name()) {
            method = {*name, std::initializer_list{class_of<TArgs>()...}, std::initializer_list{extract_type(args)...}};
        }
        auto method_info = find_method(klass, method);
        if (!method_info) {
            throw std::runtime_error("Method cannot be null");
        }
        if (!method_info->methodPointer) {
            throw std::runtime_error("Method pointer cannot be null (did you call an abstract method directly?)");
        }
        if (!method.type_checked()) {
            if (!param_match(method_info, std::initializer_list{class_of<TArgs>()...}, std::initializer_list{extract_type(args)...}).first) {
                throw std::runtime_error("Parameters do not match");
            }
            if (!is_convertible_from(type_of<T>(), method_info->return_type, false)) {
                throw std::runtime_error("Return type does not match");
            }
        }
        if (method_info->is_generic) {
            method_info = make_generic(method_info, std::initializer_list{class_of<TArgs>()...});
        }
        // Need to potentially call Class::Init here as well
        // This snippet is almost identical to what libil2cpp does
        if (method_info->flags & METHOD_ATTRIBUTE_STATIC && method_info->klass && !method_info->klass->cctor_finished_or_no_cctor) {
            functions::initialize();
            functions::Class_Init(method_info->klass);
        }
        try {
            if (method_info->flags & METHOD_ATTRIBUTE_STATIC) {
                return reinterpret_cast<function_ptr_t<T, std::decay_t<decltype(args)>..., MethodInfo const*>>(method_info->methodPointer)(
                    args..., method_info
                );
            } else {
                // In older Il2Cpp versions (not sure which), struct instances need to be boxed even for raw method pointer invokes
                void* instance = to_object<false>(class_or_inst);
                return reinterpret_cast<function_ptr_t<T, void*, std::decay_t<decltype(args)>..., MethodInfo const*>>(method_info->methodPointer)(
                    instance, args..., method_info
                );
            }
            // } catch (Il2CppExceptionWrapper& wrapper) {
            //     // logger.error(
            //     //     "{}: Failed with exception: {}", functions::method_get_name(method_info), ExceptionToString(wrapper.ex).c_str()
            //     // );
            //     // throw RunMethodException(wrapper.ex, method);
        } catch (...) {
            throw std::runtime_error("Method failed with an exception");
        }
    }
    template <type_check::valid_type T = void, type_check::valid_type... TArgs>
    T run_method(auto&& class_or_inst, find_method_info method, auto&&... args) {
        return run_method_impl<T, TArgs...>(
            {class_or_inst},
            std::forward<std::decay_t<decltype(class_or_inst)>>(class_or_inst),
            std::move(method),
            std::forward<std::decay_t<decltype(args)>>(args)...
        );
    }

    template <type_check::valid_type T>
    T get_property_impl(find_class_info klass, auto&& class_or_inst, find_property_info prop) {
        auto prop_info = THROW_UNLESS(logger, find_property(klass, prop));
        functions::initialize();
        auto getter = THROW_UNLESS(logger, functions::property_get_get_method(prop_info));
        if (!is_convertible_from(type_of<T>(), getter->return_type, false)) {
            throw std::runtime_error("Property type for getter does not match");
        }
        return run_method_impl<T>(std::move(klass), std::forward<std::decay_t<decltype(class_or_inst)>>(class_or_inst), getter);
    }
    template <type_check::valid_type T>
    T get_property(auto&& class_or_inst, find_property_info prop) {
        return get_property_impl<T>({class_or_inst}, std::forward<std::decay_t<decltype(class_or_inst)>>(class_or_inst), std::move(prop));
    }

    template <type_check::valid_type T>
    void set_property_impl(find_class_info klass, auto&& class_or_inst, find_property_info prop, T&& value) {
        auto prop_info = THROW_UNLESS(logger, find_property(klass, prop));
        functions::initialize();
        auto setter = THROW_UNLESS(logger, functions::property_get_set_method(prop_info));
        if (setter->parameters_count != 1 || !is_convertible_from(type_of<T>(), setter->parameters[0], false)) {
            throw std::runtime_error("Property type for setter does not match");
        }
        run_method_impl<T>(std::move(klass), std::forward<std::decay_t<decltype(class_or_inst)>>(class_or_inst), setter, std::forward<T>(value));
    }
    template <type_check::valid_type T>
    void set_property(auto&& class_or_inst, find_property_info prop, T&& value) {
        set_property_impl<T>(
            {class_or_inst}, std::forward<std::decay_t<decltype(class_or_inst)>>(class_or_inst), std::move(prop), std::forward<T>(value)
        );
    }

    template <type_check::valid_type T>
    T get_field_impl(find_class_info klass, auto&& class_or_inst, find_field_info field) {
        auto field_info = find_field(klass, field);
        if (!is_convertible_from(type_of<T>(), field_info->type, false)) {
            throw std::runtime_error("Field type does not match");
        }
        functions::initialize();
        auto instance = to_object<true>(class_or_inst);
        T ret;
        if (field_info->type->attrs & FIELD_ATTRIBUTE_STATIC) {
            functions::field_static_get_value(field_info, &ret);
        } else {
            functions::field_get_value(instance, field_info, &ret);
        }
        return ret;
    }
    template <type_check::valid_type T>
    T get_field(auto&& class_or_inst, find_field_info field) {
        return get_field_impl<T>({class_or_inst}, class_or_inst, std::move(field));
    }

    template <type_check::valid_type T>
    void set_field_impl(find_class_info klass, auto&& class_or_inst, find_field_info field, T&& value) {
        auto field_info = find_field(klass, field);
        if (!is_convertible_from(type_of<T>(), field_info->type, false)) {
            throw std::runtime_error("Field type does not match");
        }
        functions::initialize();
        auto instance = to_object<true>(class_or_inst);
        auto converted_value = to_object<false>(value);
        if (field_info->type->attrs & FIELD_ATTRIBUTE_STATIC) {
            functions::field_static_set_value(field_info, converted_value);
        } else {
            functions::field_set_value(instance, field_info, converted_value);
        }
    }
    template <type_check::valid_type T>
    void set_field(auto&& class_or_inst, find_field_info field, T&& value) {
        set_field_impl<T>({class_or_inst}, class_or_inst, field, std::forward<T>(value));
    }

    // Below can be considered the true APIs for the functions in this file, noting that find_x_info structs can be implicitly constructed
    // TODO: if desired, allow a result<T> type to be used as the return type, and if so use it for errors instead of throwing

    // Allocates a new instance of a particular Il2CppClass* and runs its constructor with the given arguments.
    // The class MUST be a reference type!
    // Will throw an exception if an error occurs, including not being able to find a matching constructor.
    // Can either allocate the object normally with the GC, or manually, guaranteeing its lifetime until explicitly freed.
    template <bool Manual = false>
    Il2CppObject* new_ctor(find_class_info class_info, auto&&... args) {
        auto klass = find_class(class_info);
        if (!klass) {
            throw std::runtime_error("Could not find class for new_ctor!");
        }
        Il2CppObject* obj;
        if constexpr (Manual) {
            obj = create_manual(klass);
        } else {
            functions::initialize();
            obj = functions::object_new(klass);
        }
        if (!obj) {
            throw std::runtime_error("Failed to allocate object!");
        }
        run_method(obj, ".ctor", std::forward<std::decay_t<decltype(args)>>(args)...);
        return obj;
    }

    // Allocates a new instance of a particular reference type and runs its constructor with the given arguments.
    // Will throw an exception if an error occurs, including not being able to find a matching constructor.
    // Can either allocate the object normally with the GC, or manually, guaranteeing its lifetime until explicitly freed.
    template <type_check::ref_type T, bool Manual = false>
    T new_ctor(auto&&... args) {
        Il2CppObject* inst = new_ctor<Manual>(class_of<T>(), std::forward<std::decay_t<decltype(args)>>(args)...);
        return from_object<T, false>(reinterpret_cast<void*>(inst));
    }

    // Creates a new instance of a particular value type and runs its constructor with the given arguments.
    // Will throw an exception if an error occurs, including not being able to find a matching constructor.
    template <type_check::value_type T>
    T new_ctor(auto&&... args) {
        T ret;
        run_method(ret, ".ctor", std::forward<std::decay_t<decltype(args)>>(args)...);
        return ret;
    }

    // Runs an il2cpp method on an instance or static class.
    // First template parameter is the return type, and further specified template parameters are used for generic methods.
    // (todo?) Will by default catch, log, and rethrow exceptions. i2c::result<T> can be used as the return type to capture them instead.
    // Will check types by default. A MethodInfo const* can be passed directly to the find_method_info to disable this.
    // If a MethodInfo is provided that is generic but not instantiated, it will be instantiated using TArgs.
    template <type_check::valid_type T = void, type_check::valid_type... TArgs>
    T run_method(find_class_info klass, find_method_info method, auto&&... args) {
        return run_method_impl<T, TArgs...>(std::move(klass), nullptr, std::move(method), std::forward<std::decay_t<decltype(args)>>(args)...);
    }

    // Gets a property value from an instance or static class.
    // Will throw on error, including a mismatch between T and the property type.
    template <type_check::valid_type T>
    T get_property(find_class_info klass, find_property_info prop) {
        return get_property_impl<T>(std::move(klass), nullptr, std::move(prop));
    }

    // Sets a property value to an instance or static class.
    // Will throw on error, including a mismatch between T and the property type.
    template <type_check::valid_type T>
    void set_property(find_class_info klass, find_property_info prop, T&& value) {
        set_property_impl<T>(std::move(klass), nullptr, std::move(prop), std::forward<T>(value));
    }

    // Gets a field value from an instance or static class.
    // Will throw on error, including a mismatch between T and the field type.
    template <type_check::valid_type T>
    T get_field(find_class_info klass, find_field_info field) {
        return get_field_impl<T>(std::move(klass), nullptr, std::move(field));
    }

    // Sets a field value to an instance or static class.
    // Will throw on error, including a mismatch between T and the field type.
    template <type_check::valid_type T>
    void set_field(find_class_info klass, find_field_info field, T&& value) {
        set_field_impl<T>(std::move(klass), nullptr, std::move(field), std::forward<T>(value));
    }
}
