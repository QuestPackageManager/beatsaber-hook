#include "exceptions.hpp"

#include "api.hpp"

#ifndef EXCEPTION_MESSAGE_SIZE
#define EXCEPTION_MESSAGE_SIZE 4096
#endif

std::string i2c::exception_to_string(Il2CppException const* ex) noexcept {
    functions::initialize();
    char msg[EXCEPTION_MESSAGE_SIZE];
    functions::format_exception(ex, msg, EXCEPTION_MESSAGE_SIZE);
    return msg;
}

struct backtrace_state {
    void** current;
    void** end;
    uint16_t skip;
};

static _Unwind_Reason_Code unwind_callback(struct _Unwind_Context* context, void* arg) {
    backtrace_state* state = static_cast<backtrace_state*>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        }
        if (state->skip == 0) {
            // Skip writing for the number of frames we have specified we would like to skip.
            *state->current++ = reinterpret_cast<void*>(pc);
        } else {
            state->skip--;
        }
    }
    return _URC_NO_REASON;
}

size_t i2c::capture_backtrace(void** buffer, uint16_t max, uint16_t skip) {
    backtrace_state state{buffer, buffer + max, skip};
    _Unwind_Backtrace(unwind_callback, &state);
    return state.current - buffer;
}

void i2c::trace_exception::log_backtrace() const noexcept {
    logger.error("Logging backtrace for i2c::trace_exception with size: {}...", stacktrace_size);
    logger.error("Original what(): {}", std::runtime_error::what());
    logger.error("*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***");
    logger.error("pid: {}, tid: {}", getpid(), gettid());
    for (uint16_t i = 0; i < stacktrace_size; ++i) {
        Dl_info info;
        if (dladdr(stacktrace_buffer[i], &info)) {
            // Buffer points to 1 instruction ahead
            long addr = reinterpret_cast<char*>(stacktrace_buffer[i]) - reinterpret_cast<char*>(info.dli_fbase) - 4;
            if (info.dli_sname) {
                int status;
                char const* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
                if (status) {
                    demangled = info.dli_sname;
                }
                logger.error("        #{:02}  pc {:016x}  {} ({})\n", i, addr, info.dli_fname, demangled);
                if (!status) {
                    free(const_cast<char*>(demangled));
                }
            } else {
                logger.error("        #{:02}  pc {:016x} {}\n", i, addr, info.dli_fname);
            }
        }
    }
}
