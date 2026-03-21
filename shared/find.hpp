#pragma once

#include "types.hpp"

namespace i2c {
    struct find_class_info {
        struct by_name {
            std::string_view namespaze;
            std::string_view name;
        };
        struct by_instance {
            Il2CppObject* instance;
        };
        std::variant<by_name, by_instance, Il2CppClass*> data;

        find_class_info() = delete;
        find_class_info(find_class_info&&) = default;
        find_class_info(find_class_info const&) = default;

        find_class_info& operator=(find_class_info const&) = default;

        // Finds a class by namespace and name.
        find_class_info(auto const& namespaze, auto const& name) :
            data(by_name{static_cast<std::string_view>(namespaze), static_cast<std::string_view>(name)}) {}
        // Gets the class from an instance.
        find_class_info(Il2CppObject* instance) : data(by_instance{instance}) {}
        // Passes an already found class through.
        find_class_info(Il2CppClass* klass) : data(klass) {}
        // Passes a null class through.
        find_class_info(std::nullptr_t) : data(nullptr) {}
        // Finds a class based on C++ type.
        template <type_check::valid_type T>
        find_class_info(T&&) : data(class_of<T>()) {}
    };

    struct find_method_info {
        struct by_name {
            std::string_view name;
        };
        struct by_args {
            std::string_view name;
            int args;
        };
        struct by_types {
            std::string_view name;
            std::span<Il2CppClass const* const> generics;
            std::span<Il2CppType const* const> params;
        };
        struct by_slot {
            int slot;
        };
        struct by_vtable {
            find_class_info declaring_class;
            int slot;
        };
        std::variant<by_name, by_args, by_types, by_slot, by_vtable, MethodInfo const*> data;

        find_method_info() = delete;
        find_method_info(find_method_info&&) = default;
        find_method_info(find_method_info const&) = default;

        find_method_info& operator=(find_method_info const&) = default;

        // Finds the first method with a given name.
        find_method_info(auto const& name) : data(by_name{static_cast<std::string_view>(name)}) {}
        // Finds the first method with a given name and number of arguments.
        find_method_info(auto const& name, int args) : data(by_args{static_cast<std::string_view>(name), args}) {}
        // Finds the best match of all methods with a given name, based on generics and argument types.
        find_method_info(auto const& name, std::span<Il2CppClass const* const> generics, std::span<Il2CppType const* const> params) :
            data(by_types{static_cast<std::string_view>(name), generics, params}) {}
        // Finds the method with the given vtable slot in the class.
        find_method_info(int slot) : data(by_slot{slot}) {}
        // Finds the method corresponding to the vtable slot in the given declaring class.
        find_method_info(find_class_info declaring_class, int slot) : data(by_vtable{std::move(declaring_class), slot}) {}
        // Passes an already found method through.
        find_method_info(MethodInfo const* method) : data(method) {}

        // Used to determine if a method does not need to be type checked again, in run_method.
        bool type_checked() const { return std::holds_alternative<by_types>(data) || std::holds_alternative<MethodInfo const*>(data); }

        std::optional<std::string_view> only_name() const {
            if (auto name = std::get_if<by_name>(&data)) {
                return name->name;
            }
            return std::nullopt;
        }
    };

    struct find_property_info {
        struct by_name {
            std::string_view name;
        };
        std::variant<by_name, PropertyInfo const*> data;

        find_property_info() = delete;
        find_property_info(find_property_info&&) = default;
        find_property_info(find_property_info const&) = default;

        find_property_info& operator=(find_property_info const&) = default;

        // Finds the property with a given name.
        find_property_info(auto const& name) : data(by_name{static_cast<std::string_view>(name)}) {}
        // Passes an already found property through.
        find_property_info(PropertyInfo const* prop) : data(prop) {}
    };

    struct find_field_info {
        struct by_name {
            std::string_view name;
        };
        std::variant<by_name, FieldInfo*> data;

        find_field_info() = delete;
        find_field_info(find_field_info&&) = default;
        find_field_info(find_field_info const&) = default;

        find_field_info& operator=(find_field_info const&) = default;

        // Finds the field with a given name.
        find_field_info(auto const& name) : data(by_name{static_cast<std::string_view>(name)}) {}
        // Passes an already found field through.
        find_field_info(FieldInfo* field) : data(field) {}
    };

    // Finds a class based on the find_class_info. (Cached)
    Il2CppClass* find_class(find_class_info const& info);
    // Finds a method in the given class, based on the find_method_info. (Cached)
    MethodInfo const* find_method(find_class_info const& class_info, find_method_info const& info);
    // Finds a property in the given class, based on the find_property_info. (Cached)
    PropertyInfo const* find_property(find_class_info const& class_info, find_property_info const& info);
    // Finds a field in the given class, based on the find_field_info. (Cached)
    FieldInfo* find_field(find_class_info const& class_info, find_field_info const& info);

    // Represents a specialization type that should be used for exposing metadata from particular values, such as methods.
    template <auto V>
    struct BS_HOOK_HIDDEN metadata_getter;

    template <auto V>
    concept valid_method = requires {
        { metadata_getter<V>::method_info() } -> std::same_as<MethodInfo const*>;
    };
}
