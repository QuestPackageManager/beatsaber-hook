#pragma once

#include "types.hpp"

namespace i2c {
    // Returns a legible string from an Il2CppException*
    std::string exception_to_string(Il2CppException const* ex) noexcept;
}
