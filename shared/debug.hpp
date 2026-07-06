#pragma once

#include "api.hpp"

namespace i2c {
    // Some parts provided by zoller27osu
    // Logs information about the given Il2CppClass* as log(DEBUG)
    void log_class(Paper::LoggerContext const& logger, Il2CppClass* klass, bool parents = false) noexcept;

    // Logs all classes (from every namespace) that start with the given prefix
    // WARNING: THIS FUNCTION IS VERY SLOW. ONLY USE THIS FUNCTION ONCE AND WITH A FAIRLY SPECIFIC PREFIX!
    void log_classes(Paper::LoggerContext const& logger, std::string_view prefix, bool parents = false) noexcept;

    // Function made by zoller27osu, modified by Sc2ad
    // Logs information about the given MethodInfo* as log(DEBUG)
    void log_method(Paper::LoggerContext const& logger, MethodInfo const* method);

    // Created by zoller27osu
    // Calls LogMethod on all methods in the given class
    void log_methods(Paper::LoggerContext const& logger, Il2CppClass const* klass, bool parents = false);

    // Created by zoller27osu
    // Logs information about the given FieldInfo* as log(DEBUG)
    void log_field(Paper::LoggerContext const& logger, FieldInfo* field);

    // Created by zoller27osu
    // Calls LogField on all fields in the given class
    void log_fields(Paper::LoggerContext const& logger, Il2CppClass* klass, bool parents = false);

    // Created by zoller27osu
    // Logs information about the given PropertyInfo* as log(DEBUG)
    void log_property(Paper::LoggerContext const& logger, PropertyInfo const* prop);

    // Created by zoller27osu
    // Calls LogProperty on all properties in the given class
    void log_properties(Paper::LoggerContext const& logger, Il2CppClass* klass, bool parents = false);
}
