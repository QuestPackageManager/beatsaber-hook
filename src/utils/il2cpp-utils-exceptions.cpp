#include "../../shared/utils/il2cpp-utils-exceptions.hpp"
#include "../../shared/utils/il2cpp-functions.hpp"

namespace il2cpp_utils {
    // Init all of the usable il2cpp API, if it has yet to be initialized
    // Maximum length of characters of an exception message - 1
    #ifndef EXCEPTION_MESSAGE_SIE
    #define EXCEPTION_MESSAGE_SIZE 4096
    #endif
    // Returns a legible string from an Il2CppException*
    std::string ExceptionToString(const Il2CppException* exp) noexcept {
        il2cpp_functions::Init();

        char msg[EXCEPTION_MESSAGE_SIZE];
        il2cpp_functions::format_exception(exp, msg, EXCEPTION_MESSAGE_SIZE);
        // auto exception_message = csstrtostr(exp->message);
        // return to_utf8(exception_message);
        return msg;
    }

    #if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    [[noreturn]] void raise(const Il2CppException* exp) {
        CRASH_UNLESS(exp);
        il2cpp_functions::raise_exception(const_cast<Il2CppException*>(exp));
        // Should never get here, since the exception raise should happen and thus we should no longer be the caller.
        __builtin_unreachable();
    }
    #endif

    // TODO: Add a logger argument here so we could better write out to a targetted buffer.
    // For now, we will stick to using the UtilsLogger.
    // It will be our caller's responsibility to determine what to do AFTER the backtrace is logged-- whether it be to terminate or rethrow.
    // Logs the backtrace with the Logging::ERROR level, using the global logger instance.
    static void log_backtrace_full(void* const* stacktrace_buffer, uint16_t stacktrace_size) {
        auto const& logger = il2cpp_utils::Logger;
        logger.error("[UNCAUGHT-EXCEPTION] Logging backtrace for RunMethodException with size: {}...", stacktrace_size);
        logger.error("[UNCAUGHT-EXCEPTION] *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***");
        logger.error("[UNCAUGHT-EXCEPTION] pid: {}, tid: {}", getpid(), gettid());
        for (uint16_t i = 0; i < stacktrace_size; ++i) {
            Dl_info info;
            if (dladdr(stacktrace_buffer[i], &info)) {
                // Buffer points to 1 instruction ahead
                long addr = reinterpret_cast<char*>(stacktrace_buffer[i]) - reinterpret_cast<char*>(info.dli_fbase) - 4;
                if (info.dli_sname) {
                    int status;
                    const char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
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

    void exceptions::StackTraceException::log_backtrace() const {
        log_backtrace_full(stacktrace_buffer, stacktrace_size);
    }

    void RunMethodException::log_backtrace() const {
        log_backtrace_full(stacktrace_buffer, stacktrace_size);
    }
}
