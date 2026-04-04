#pragma once

#include "config.hpp"

#include <source_location>

// Forward declare so this can be included before Il2Cpp includes
struct Il2CppException;

namespace i2c {
    // Returns a legible string from an Il2CppException*
    std::string exception_to_string(Il2CppException const* ex) noexcept;

    size_t capture_backtrace(void** buffer, uint16_t max, uint16_t skip = 0);

    struct trace_exception : std::runtime_error {
        constexpr static uint16_t STACK_TRACE_SIZE = 256;

        void* stacktrace_buffer[STACK_TRACE_SIZE];
        uint16_t stacktrace_size;

        trace_exception(std::string_view msg, std::source_location sl = std::source_location::current()) :
            std::runtime_error(fmt::format("[{}:{}:{} @ {}] {}", sl.file_name(), sl.line(), sl.column(), sl.function_name(), msg)) {
            // TODO: Eventually skip two frames (assuming no inlined methods) for this constructor and the captured backtrace call.
            stacktrace_size = capture_backtrace(stacktrace_buffer, STACK_TRACE_SIZE, 0);
        }
        void log_backtrace() const noexcept;

        [[nodiscard]] virtual char const* what() const noexcept override {
            log_backtrace();
            return std::runtime_error::what();
        }
    };
}
