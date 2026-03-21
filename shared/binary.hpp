#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace i2c::binary {
    uintptr_t get_base(void* pc);
    ptrdiff_t as_offset(void* pc);

    // Attempts to print what is stored at the given pointer.
    // For a given pointer, it will scan 4 void*'s worth of bytes at the location pointed to.
    // For each void* of bytes, it will print the raw bytes and interpretations of the bytes as ints and char*s.
    // When the bytes look like a valid pointer, it will attempt to follow that pointer, increasing the indentation.
    //   It will not follow pointers that it has already analyzed as a result of the current call.
    void analyze_bytes(void const* ptr);

    // Dumps the 'before' bytes before and 'after' bytes after the given pointer to log
    void dump(int before, int after, void* ptr);

    uintptr_t get_real_offset(void const* offset);
    uintptr_t base_addr(char const* soname);

    // Only wildcard is ? and ?? - both are handled the same way. They will skip exactly 1 byte (2 hex digits)
    uintptr_t find_pattern(uintptr_t dw_addr, char const* pattern, uintptr_t dw_search_len);
    // Same as find_pattern but will continue scanning to make sure your pattern is sufficiently specific.
    // Each candidate will be logged. label should describe what you're looking for, like "Class::Init".
    // Sets "multiple" iff multiple matches are found, and outputs a log warning message.
    // Returns the first match, if any.
    uintptr_t find_unique_pattern(bool& multiple, uintptr_t dw_addr, char const* pattern, uintptr_t dw_search_len, char const* label = 0);

    /// @brief Attempts to match the pattern provided with all regions of mapped read memory with the file provided
    uintptr_t mapped_file_unique_pattern(bool& multiple, char const* pattern, char const* file, char const* label = 0);

    /// @brief Attempts to match the pattern provided with all regions of mapped read memory with the libil2cpp.so
    uintptr_t libil2cpp_unique_pattern(bool& multiple, char const* pattern, char const* label = 0);

    /// @brief Attempts to match the pattern provided with all regions of mapped read memory with the libunity.so
    uintptr_t libunity_unique_pattern(bool& multiple, char const* pattern, char const* label = 0);

    /// @brief Get the size of the libil2cpp.so file
    uintptr_t get_libil2cpp_size();

    /// @brief Get the build id from a file
    std::optional<std::string> get_build_id(std::string_view filename);
}
