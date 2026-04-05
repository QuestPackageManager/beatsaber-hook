#pragma once

#include "api.hpp"
#include "exceptions.hpp"
#include "find.hpp"
#include "types.hpp"

namespace i2c {
    // The purpose of the separate implementation and overloads is to preserve the compile time type of class_or_inst,
    // while still allowing for the elegant construction of find_class_info with multiple parameters (namepace + name)
    namespace detail {
        template <type_check::full_type T = void, type_check::has_type... TArgs>
        T run_method_impl(find_class_info klass, auto&& class_or_inst, find_method_info method, auto&&... args) {
            MethodInfo const* method_info;
            bool types_checked = method.type_checked();
            if (std::optional<std::string_view> name = method.only_name()) {
                types_checked = true;
                method_info = find_method(klass, {*name, {class_of<TArgs>()...}, {extract_type(args)...}});
            } else {
                method_info = find_method(klass, method);
            }
            if (!method_info) {
                throw i2c::trace_exception("Method cannot be null");
            }
            if (!method_info->methodPointer) {
                throw i2c::trace_exception("Method pointer cannot be null (did you call an abstract method directly?)");
            }
            if (!types_checked) {
                if (param_match(method_info, {class_of<TArgs>()...}, {extract_type(args)...}) == match::none) {
                    throw i2c::trace_exception("Parameters do not match");
                }
                if (!is_convertible_from(type_of<T>(), method_info->return_type, false)) {
                    throw i2c::trace_exception("Return type does not match");
                }
            }
            if (method_info->is_generic) {
                method_info = make_generic(method_info, {class_of<TArgs>()...});
            }
            // Invoke the method with runtime_invoke - using the method pointer would be more performant,
            // but much more effort to catch and handle exceptions with their info
            functions::initialize();
            void* inst = method_info->flags & METHOD_ATTRIBUTE_STATIC ? nullptr : to_object<false>(class_or_inst);
            std::array<void*, sizeof...(args)> params{to_object<false>(args)...};
            Il2CppException* ex = nullptr;
            Il2CppObject* ret = functions::runtime_invoke(method_info, inst, params.data(), &ex);
            if (ex) {
                throw i2c::trace_exception(fmt::format("Method: {} failed with an exception: {}", method_info->name, exception_to_string(ex)));
            }
            if constexpr (!std::is_void_v<T>) {
                if constexpr (type_check::value_type<T>) {
                    on_scope_exit f([ret]() { functions::GC_free(ret); });
                    return from_object<T, true>(ret);
                } else {
                    return from_object<T, true>(ret);
                }
            }
        }

        template <type_check::full_type T>
        T get_property_impl(find_class_info klass, auto&& class_or_inst, find_property_info prop) {
            auto prop_info = THROW_UNLESS(logger, find_property(klass, prop));
            functions::initialize();
            auto getter = THROW_UNLESS(logger, functions::property_get_get_method(prop_info));
            if (!is_convertible_from(type_of<T>(), getter->return_type, false)) {
                throw i2c::trace_exception("Property type for getter does not match");
            }
            return run_method_impl<T>(std::move(klass), std::forward<decltype(class_or_inst)>(class_or_inst), getter);
        }

        template <type_check::full_type T>
        void set_property_impl(find_class_info klass, auto&& class_or_inst, find_property_info prop, T&& value) {
            auto prop_info = THROW_UNLESS(logger, find_property(klass, prop));
            functions::initialize();
            auto setter = THROW_UNLESS(logger, functions::property_get_set_method(prop_info));
            if (setter->parameters_count != 1 || !is_convertible_from(setter->parameters[0], type_of<T>(), true)) {
                throw i2c::trace_exception("Property type for setter does not match");
            }
            run_method_impl<T>(std::move(klass), std::forward<decltype(class_or_inst)>(class_or_inst), setter, std::forward<T>(value));
        }

        template <type_check::full_type T>
        T get_field_impl(find_class_info klass, auto&& class_or_inst, find_field_info field) {
            auto field_info = find_field(klass, field);
            if (!is_convertible_from(type_of<T>(), field_info->type, false)) {
                throw i2c::trace_exception("Field type does not match");
            }
            functions::initialize();
            T ret;
            if (field_info->type->attrs & FIELD_ATTRIBUTE_STATIC) {
                functions::field_static_get_value(field_info, &ret);
            } else {
                auto instance = to_object<true>(class_or_inst);
                functions::field_get_value(instance, field_info, &ret);
            }
            return ret;
        }

        template <type_check::full_type T>
        void set_field_impl(find_class_info klass, auto&& class_or_inst, find_field_info field, T&& value) {
            auto field_info = find_field(klass, field);
            if (!is_convertible_from(field_info->type, type_of<T>(), false)) {
                throw i2c::trace_exception("Field type does not match");
            }
            functions::initialize();
            auto converted_value = to_object<false>(value);
            if (field_info->type->attrs & FIELD_ATTRIBUTE_STATIC) {
                functions::field_static_set_value(field_info, converted_value);
            } else {
                auto instance = to_object<true>(class_or_inst);
                functions::field_set_value(instance, field_info, converted_value);
            }
        }
    }

    // The following functions only exist to help argument deduction (see the top of the file for more info).
    // Scroll down a bit for the documentation and API.
    template <type_check::full_type T = void, type_check::has_type... TArgs>
    T run_method(auto&& class_or_inst, find_method_info method, auto&&... args) {
        return detail::run_method_impl<T, TArgs...>(
            {class_or_inst}, std::forward<decltype(class_or_inst)>(class_or_inst), std::move(method), std::forward<decltype(args)>(args)...
        );
    }

    template <type_check::full_type T>
    T get_property(auto&& class_or_inst, find_property_info prop) {
        return detail::get_property_impl<T>({class_or_inst}, std::forward<decltype(class_or_inst)>(class_or_inst), std::move(prop));
    }

    template <type_check::full_type T>
    void set_property(auto&& class_or_inst, find_property_info prop, T&& value) {
        detail::set_property_impl<T>({class_or_inst}, std::forward<decltype(class_or_inst)>(class_or_inst), std::move(prop), std::forward<T>(value));
    }

    template <type_check::full_type T>
    T get_field(auto&& class_or_inst, find_field_info field) {
        return detail::get_field_impl<T>({class_or_inst}, std::forward<decltype(class_or_inst)>(class_or_inst), std::move(field));
    }

    template <type_check::full_type T>
    void set_field(auto&& class_or_inst, find_field_info field, T&& value) {
        detail::set_field_impl<T>({class_or_inst}, std::forward<decltype(class_or_inst)>(class_or_inst), std::move(field), std::forward<T>(value));
    }

    // Below can be considered the true APIs for the functions in this file, noting that find_x_info structs can be implicitly constructed
    // TODO: if desired, allow a result<T> type to be used as the return type, and if so use it for errors instead of throwing

    // Runs an il2cpp method on an instance or static class.
    // First template parameter is the return type, and further specified template parameters are used for generic methods.
    // (todo?) Will by default catch, log, and rethrow exceptions. i2c::result<T> can be used as the return type to capture them instead.
    // Will check types by default. A MethodInfo const* can be passed directly to the find_method_info to disable this.
    // If a MethodInfo is provided that is generic but not instantiated, it will be instantiated using TArgs.
    template <type_check::full_type T = void, type_check::has_type... TArgs>
    T run_method(find_class_info klass, find_method_info method, auto&&... args) {
        return detail::run_method_impl<T, TArgs...>(std::move(klass), nullptr, std::move(method), std::forward<decltype(args)>(args)...);
    }

    // Gets a property value from an instance or static class.
    // Will throw on error, including a mismatch between T and the property type.
    template <type_check::full_type T>
    T get_property(find_class_info klass, find_property_info prop) {
        return detail::get_property_impl<T>(std::move(klass), nullptr, std::move(prop));
    }

    // Sets a property value to an instance or static class.
    // Will throw on error, including a mismatch between T and the property type.
    template <type_check::full_type T>
    void set_property(find_class_info klass, find_property_info prop, T&& value) {
        detail::set_property_impl<T>(std::move(klass), nullptr, std::move(prop), std::forward<T>(value));
    }

    // Gets a field value from an instance or static class.
    // Will throw on error, including a mismatch between T and the field type.
    template <type_check::full_type T>
    T get_field(find_class_info klass, find_field_info field) {
        return detail::get_field_impl<T>(std::move(klass), nullptr, std::move(field));
    }

    // Sets a field value to an instance or static class.
    // Will throw on error, including a mismatch between T and the field type.
    template <type_check::full_type T>
    void set_field(find_class_info klass, find_field_info field, T&& value) {
        detail::set_field_impl<T>(std::move(klass), nullptr, std::move(field), std::forward<T>(value));
    }

    // Allocates a new instance of a particular Il2CppClass* and runs its constructor with the given arguments.
    // The class MUST be a reference type!
    // Will throw an exception if an error occurs, including not being able to find a matching constructor.
    // Can either allocate the object normally with the GC, or manually, guaranteeing its lifetime until explicitly freed.
    template <bool Manual = false>
    Il2CppObject* new_ctor(find_class_info class_info, auto&&... args) {
        auto klass = find_class(class_info);
        if (!klass) {
            throw i2c::trace_exception("Could not find class for new_ctor!");
        }
        Il2CppObject* obj;
        if constexpr (Manual) {
            obj = create_manual(klass);
        } else {
            functions::initialize();
            obj = functions::object_new(klass);
        }
        if (!obj) {
            throw i2c::trace_exception("Failed to allocate object!");
        }
        run_method(obj, ".ctor", std::forward<decltype(args)>(args)...);
        return obj;
    }

    // Allocates a new instance of a particular reference type and runs its constructor with the given arguments.
    // Will throw an exception if an error occurs, including not being able to find a matching constructor.
    // Can either allocate the object normally with the GC, or manually, guaranteeing its lifetime until explicitly freed.
    template <type_check::ref_type T, bool Manual = false>
    T new_ctor(auto&&... args) {
        Il2CppObject* inst = new_ctor<Manual>(class_of<T>(), std::forward<decltype(args)>(args)...);
        return from_object<T, false>(reinterpret_cast<void*>(inst));
    }

    // Creates a new instance of a particular value type and runs its constructor with the given arguments.
    // Will throw an exception if an error occurs, including not being able to find a matching constructor.
    template <type_check::value_type T>
    T new_ctor(auto&&... args) {
        T ret;
        run_method(ret, ".ctor", std::forward<decltype(args)>(args)...);
        return ret;
    }
}
