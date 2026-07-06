#include "binary.hpp"

#include "config.hpp"
#include "utils.hpp"

#include <dlfcn.h>
#include <link.h>
#include <sys/stat.h>

uintptr_t i2c::binary::get_base(void* pc) {
    Dl_info info;
    RET_DEF_UNLESS(logger, dladdr(pc, &info));
    return (uintptr_t) info.dli_fbase;
}
ptrdiff_t i2c::binary::as_offset(void* pc) {
    auto base = get_base(pc);
    return (ptrdiff_t) (((uintptr_t) pc) - base);
}

static std::unordered_set<void const*> analyzed;

static void internal_analyze_bytes(std::stringstream& ss, void const* ptr, int indent) {
    if (!ptr || ((uintptr_t const) ptr) > 0x7fffffffffll) {
        return;
    }

    i2c::tabs(ss, indent);
    if (analyzed.contains(ptr)) {
        ss << "! loop at 0x" << std::hex << ptr << "!";
        i2c::logger.info("{}", ss.str());
        return;
    }
    analyzed.insert(ptr);

    auto as_uints = reinterpret_cast<uintptr_t const*>(ptr);
    auto as_ints = reinterpret_cast<intptr_t const*>(ptr);
    auto as_chars = reinterpret_cast<char const*>(ptr);
    if (as_uints[0] >= 0x1000000000000ll && isprint(as_chars[0])) {
        ss << "chars: \"" << as_chars << "\"";
        ss << " (first 8 bytes in hex = 0x" << std::hex << std::setw(16) << as_uints[0] << ")";
        i2c::logger.info("{}", ss.str());
        return;
    }
    for (int i = 0; i < 4; i++) {
        if (i != 0) {
            i2c::tabs(ss, indent);
        }
        ss << "pos " << std::dec << i << ": 0x" << std::hex << std::setw(16) << as_uints[i];
        if (as_uints[i] >= 0x8000000000ll) {
            // todo: read no more than 8 chars or move asInts to last aligned point in string
            ss << " (as chars = \"" << reinterpret_cast<char const*>(as_uints + i) << "\")";
            ss << " (as int = " << std::dec << as_ints[i] << ")";  // signed int
        } else if (as_uints[i] <= 0x7f00000000ll) {
            ss << " (as int = " << std::dec << as_uints[i] << ")";
        } else {
            Dl_info inf;
            if (dladdr((void*) as_uints[i], &inf)) {
                ss << " (dli_fname: " << inf.dli_fname << ", dli_fbase: " << std::hex << std::setw(16) << (uintptr_t) inf.dli_fbase;
                ss << ", offset = 0x" << std::hex << std::setw(8) << (as_uints[i] - (uintptr_t) inf.dli_fbase);
                if (inf.dli_sname) {
                    ss << ", dli_sname: " << inf.dli_sname << ", dli_saddr: " << std::hex << std::setw(16) << (uintptr_t) inf.dli_saddr;
                }
                ss << ")";
            }
        }
        i2c::logger.info("{}", ss.str());
        if (as_uints[i] > 0x7f00000000ll) {
            internal_analyze_bytes(ss, (void*) as_uints[i], indent + 1);
        }
    }
}

void i2c::binary::analyze_bytes(void const* ptr) {
    analyzed.clear();
    std::stringstream ss;
    ss << std::setfill('0');
    ss << "ptr: " << std::hex << std::setw(16) << (uintptr_t) ptr;
    logger.info("{}", ss.str());
    internal_analyze_bytes(ss, ptr, 0);
}

void i2c::binary::dump(int before, int after, void* ptr) {
    logger.debug("Dumping immediate pointer: {}: {:08x}", fmt::ptr(ptr), *reinterpret_cast<int*>(ptr));
    auto begin = static_cast<int*>(ptr) - before;
    auto end = static_cast<int*>(ptr) + after;
    for (auto cur = begin; cur != end; ++cur) {
        logger.debug("{}: {:08x}", fmt::ptr(cur), *cur);
    }
}

uintptr_t location;  // save libil2cpp.so base address so we do not have to recalculate every time

uintptr_t i2c::binary::get_real_offset(void const* offset) {
    if (location == 0) {
        location = base_addr(modloader_get_libil2cpp_path());
    }
    return location + (uintptr_t) offset;
}

struct bdata {
    uintptr_t base;
    char const* soname;
};

static int iterate_base_addr(dl_phdr_info* info, size_t, void* data) {
    auto dat = reinterpret_cast<bdata*>(data);
    if (std::string_view(info->dlpi_name).find(dat->soname) == std::string::npos) {
        return 0;
    }
    dat->base = (uintptr_t) info->dlpi_addr;
    return 1;
}

uintptr_t i2c::binary::base_addr(char const* soname) {
    if (soname == nullptr) {
        return 0;
    }
    bdata dat;
    dat.soname = soname;
    int status = dl_iterate_phdr(iterate_base_addr, &dat);
    if (status) {
        return dat.base;
    }
    logger.error("base_addr: Error on dl_iterate_phdr!");
    return 0;
}

#define in_range(x, a, b) (x >= a && x <= b)
#define get_bits(x) (in_range((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA) : (in_range(x, '0', '9') ? x - '0' : 0))
#define get_byte(x) (get_bits(x[0]) << 4 | get_bits(x[1]))

uintptr_t i2c::binary::find_pattern(uintptr_t dw_addr, char const* pattern, uintptr_t dw_search_len) {
    // To avoid a lot of bad match candidates, pre-process wildcards at the front of the pattern
    uintptr_t skipped_start_bytes = 0;
    while (pattern[0] == '\?') {
        // see comments below for insight on these numbers
        pattern += (pattern[1] == '\?') ? 3 : 2;
        skipped_start_bytes++;
    }
    uintptr_t match = 0;  // current match candidate
    uintptr_t len = strlen(CRASH_UNLESS(pattern));
    if (dw_search_len < len) {
        return 0;
    }
    char const* pat = pattern;  // current spot in the pattern

    for (uintptr_t p_cur = dw_addr + skipped_start_bytes; p_cur < dw_addr + dw_search_len; p_cur++) {
        // If pat[0] is null char, we are done, or if pat >= pattern + len
        if (pat >= pattern + len || !pat[0] || !pat[1]) {
            return match;
        }
        // For each byte, if the pattern starts with a ? or the current byte matches:
        if (pat[0] == '\?' || *(char*) p_cur == get_byte(pat)) {
            // If we do not have a match, begin it
            if (!match) {
                match = p_cur - skipped_start_bytes;
            }
            // If our next character is at the end of our pattern, we have a match
            if (pat + 1 >= pattern + len) {
                return match;
            }
            if (pat[0] != '\?' || pat[1] == '\?') {
                pat += 3;  // advance past "xy " or "?? "
            } else {
                pat += 2;  // advance past "? "
            }
        } else {
            // reset search position to beginning of the failed match; for loop will begin new search at match + 1
            if (match) {
                p_cur = match + skipped_start_bytes;
            }
            pat = pattern;
            match = 0;
        }
    }
    return 0;
}
uintptr_t i2c::binary::find_unique_pattern(bool& multiple, uintptr_t dw_addr, char const* pattern, uintptr_t dw_search_len, char const* label) {
    uintptr_t first_match_addr = 0;
    uintptr_t new_match_addr;
    uintptr_t start = dw_addr;
    uintptr_t end = dw_addr + dw_search_len;
    int matches = 0;
    logger.debug("Sigscan for pattern: {}", pattern);
    while (start > 0 && start < end && (new_match_addr = find_pattern(start, pattern, end - start))) {
        if (!first_match_addr) {
            first_match_addr = new_match_addr;
        }
        matches++;
        if (label) {
            logger.debug("Sigscan found possible \"{}\": offset 0x{:x}, pointer 0x{:x}", label, new_match_addr - dw_addr, new_match_addr);
        }
        start = new_match_addr + 1;
        logger.debug("start = 0x{:x}, end = 0x{:x}", start, end);
        usleep(1000);
    }
    if (matches > 1) {
        multiple = true;
        logger.warn("Multiple sig scan matches for \"{}\"!", label);
    }
    return first_match_addr;
}

uintptr_t i2c::binary::mapped_file_unique_pattern(bool& multiple, char const* pattern, char const* file, char const* label) {
    // Essentially call find_unique_pattern for each segment listed in /proc/self/maps
    std::ifstream proc_map("/proc/self/maps");
    std::string line;
    uintptr_t first_match_addr = 0;
    while (std::getline(proc_map, line)) {
        if (line.find(file) == std::string::npos) {
            continue;
        }
        auto idx = line.find_first_of('-');
        CRASH_UNLESS(idx != std::string::npos);
        auto start_addr = std::stoul(line.substr(0, idx), nullptr, 16);
        auto space_idx = line.find_first_of(' ');
        CRASH_UNLESS(space_idx != std::string::npos);
        auto end_addr = std::stoul(line.substr(idx + 1, space_idx - idx - 1), nullptr, 16);
        // Permissions are 4 characters
        auto perms = line.substr(space_idx + 1, 4);
        if (perms.find('r') != std::string::npos) {
            // Search between start and end
            uintptr_t match = find_unique_pattern(multiple, start_addr, pattern, end_addr - start_addr, label);
            if (!first_match_addr) {
                first_match_addr = match;
            }
        }
    }
    proc_map.close();
    return first_match_addr;
}

uintptr_t i2c::binary::libil2cpp_unique_pattern(bool& multiple, char const* pattern, char const* label) {
    return mapped_file_unique_pattern(multiple, pattern, "libil2cpp.so", label);
}

uintptr_t i2c::binary::libunity_unique_pattern(bool& multiple, char const* pattern, char const* label) {
    return mapped_file_unique_pattern(multiple, pattern, "libunity.so", label);
}

static uintptr_t cached_libil2cpp_size = 0;

uintptr_t i2c::binary::get_libil2cpp_size() {
    if (cached_libil2cpp_size == 0) {
        struct stat st;
        if (!stat(modloader_get_libil2cpp_path(), &st)) {
            cached_libil2cpp_size = st.st_size;
        }
        logger.debug("libil2cpp.so size: 0x{:X}", cached_libil2cpp_size);
    }
    return cached_libil2cpp_size;
}

std::optional<std::string> i2c::binary::get_build_id(std::string_view filename) {
    std::ifstream infile(filename.data(), std::ios_base::binary);
    if (!infile.is_open()) {
        return std::nullopt;
    }
    infile.seekg(0);
    ElfW(Ehdr) elf;
    infile.read(reinterpret_cast<char*>(&elf), sizeof(ElfW(Ehdr)));
    for (int i = 0; i < elf.e_shnum; i++) {
        infile.seekg(elf.e_shoff + i * elf.e_shentsize);
        ElfW(Shdr) section;
        infile.read(reinterpret_cast<char*>(&section), sizeof(ElfW(Shdr)));
        if (section.sh_type != SHT_NOTE || section.sh_size != 0x24) {
            continue;
        }
        char data[0x24];
        infile.seekg(section.sh_offset);
        infile.read(data, 0x24);
        ElfW(Nhdr)* note = reinterpret_cast<ElfW(Nhdr)*>(data);
        if (note->n_namesz != 4 || note->n_descsz != 20) {
            continue;
        }
        if (memcmp(reinterpret_cast<void*>(data + 12), "GNU", 4) != 0) {
            continue;
        }
        std::stringstream stream;
        stream << std::hex << std::setw(sizeof(uint8_t) * 2);
        auto build_id_addr = reinterpret_cast<uint8_t*>(data + 16);
        for (int i = 0; i < 5; i++) {
            uint32_t value;
            auto ptr = (reinterpret_cast<uint8_t*>(&value));
            ptr[0] = *(build_id_addr + i * sizeof(uint32_t) + 3);
            ptr[1] = *(build_id_addr + i * sizeof(uint32_t) + 2);
            ptr[2] = *(build_id_addr + i * sizeof(uint32_t) + 1);
            ptr[3] = *(build_id_addr + i * sizeof(uint32_t));
            stream << value;
        }
        return stream.str();
    }
    return std::nullopt;
}
