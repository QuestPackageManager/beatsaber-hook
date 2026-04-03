#include "exceptions.hpp"

#ifndef EXCEPTION_MESSAGE_SIE
#define EXCEPTION_MESSAGE_SIZE 4096
#endif

std::string i2c::exception_to_string(Il2CppException const* ex) noexcept {
    functions::initialize();
    char msg[EXCEPTION_MESSAGE_SIZE];
    functions::format_exception(ex, msg, EXCEPTION_MESSAGE_SIZE);
    return msg;
}
