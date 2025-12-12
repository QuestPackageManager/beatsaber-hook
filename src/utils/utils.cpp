// thx https://github.com/jbro129/Unity-Substrate-Hook-Android
#include "../../shared/utils/utils.h"
#include <sys/stat.h>
#include <sys/types.h>

#include <dlfcn.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <link.h>
#include "il2cpp-object-internals.h"
#include "shared/utils/gc-alloc.hpp"
#include "utils/logging.hpp"

namespace backtrace_helpers {
_Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg) {
    BacktraceState* state = static_cast<BacktraceState*>(arg);
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
size_t captureBacktrace(void** buffer, uint16_t max, uint16_t skip) {
    BacktraceState state{ buffer, buffer + max, skip };
    _Unwind_Backtrace(unwindCallback, &state);

    return state.current - buffer;
}
}  // namespace backtrace_helpers


void resetSS(std::stringstream& ss) {
    ss.str("");
    ss.clear();  // Clear state flags.
}

void tabs(std::ostream& os, int tabs, int spacesPerTab) {
    for (int i = 0; i < tabs * spacesPerTab; i++) {
        os << " ";
    }
}


static std::unordered_set<const void*> analyzed;
void analyzeBytes(std::stringstream& ss, const void* ptr, int indent) {
    if (!ptr || ((const uintptr_t)ptr) > 0x7fffffffffll) return;

    tabs(ss, indent);
    if (analyzed.count(ptr)) {
        ss << "! loop at 0x" << std::hex << ptr << "!";
        il2cpp_utils::Logger.info("{}", ss.str());
        return;
    }
    analyzed.insert(ptr);

    auto asUInts = reinterpret_cast<const uintptr_t*>(ptr);
    auto asInts = reinterpret_cast<const intptr_t*>(ptr);
    auto asChars = reinterpret_cast<const char*>(ptr);
    if (asUInts[0] >= 0x1000000000000ll && isprint(asChars[0])) {
        ss << "chars: \"" << asChars << "\"";
        ss << " (first 8 bytes in hex = 0x" << std::hex << std::setw(16) << asUInts[0] << ")";
        il2cpp_utils::Logger.info("{}", ss.str());
        return;
    }
    for (int i = 0; i < 4; i++) {
        if (i != 0) tabs(ss, indent);

        ss << "pos " << std::dec << i << ": 0x" << std::hex << std::setw(16) << asUInts[i];
        if (asUInts[i] >= 0x8000000000ll) {
            // todo: read no more than 8 chars or move asInts to last aligned point in string
            ss << " (as chars = \"" << reinterpret_cast<const char*>(asUInts + i) << "\")";
            ss << " (as int = " << std::dec << asInts[i] << ")";  // signed int
        } else if (asUInts[i] <= 0x7f00000000ll) {
            ss << " (as int = " << std::dec << asUInts[i] << ")";
        } else {
            Dl_info inf;
            if (dladdr((void*)asUInts[i], &inf)) {
                ss << " (dli_fname: " << inf.dli_fname << ", dli_fbase: " << std::hex << std::setw(16) << (uintptr_t)inf.dli_fbase;
                ss << ", offset = 0x" << std::hex << std::setw(8) << (asUInts[i] - (uintptr_t)inf.dli_fbase);
                if (inf.dli_sname) {
                    ss << ", dli_sname: " << inf.dli_sname << ", dli_saddr: " << std::hex << std::setw(16) << (uintptr_t)inf.dli_saddr;
                }
                ss << ")";
            }
        }
        il2cpp_utils::Logger.info("{}", ss.str());
        if (asUInts[i] > 0x7f00000000ll) {
            analyzeBytes(ss, (void*)asUInts[i], indent + 1);
        }
    }
}

static uintptr_t soSize = 0;

uintptr_t getLibil2cppSize() {
    auto const& contextLogger = il2cpp_utils::Logger;
    if (soSize == 0) {
        struct stat st;
        if (!stat(modloader_get_libil2cpp_path(), &st)) {
            soSize = st.st_size;
        }
        contextLogger.debug("libil2cpp.so size: 0x{:X}", soSize);
    }
    return soSize;
}


void analyzeBytes(const void* ptr) {
    analyzed.clear();
    std::stringstream ss;
    ss << std::setfill('0');
    ss << "ptr: " << std::hex << std::setw(16) << (uintptr_t)ptr;
    il2cpp_utils::Logger.info("{}", ss.str());
    analyzeBytes(ss, ptr, 0);
}

uintptr_t baseAddr(const char* soname) {
    if (soname == NULL) return (uintptr_t)NULL;
    struct bdata {
      uintptr_t base;
      const char* soname;
    };
    bdata dat;
    dat.soname = soname;
    int status = dl_iterate_phdr([] (dl_phdr_info* info, size_t, void* data) {
        bdata* dat = reinterpret_cast<bdata*>(data);
        if (std::string(info->dlpi_name).find(dat->soname) != std::string::npos) {
          dat->base = (uintptr_t)info->dlpi_addr;
          return 1;
        }
        return 0;
    }, &dat);
    if(status)
      return dat.base;
    il2cpp_utils::Logger.error("baseAddr: Error on dl_iterate_phdr!");
    return (uintptr_t)NULL;
}

uintptr_t location;  // save lib.so base address so we do not have to recalculate every time causing lag.

uintptr_t getRealOffset(const void* offset)  // calculate dump.cs address + lib.so base address.
{
    if (location == 0) {
        // arm
        //  TOOD: Lets get the instance via some sort of initialization function
        //  OR we make EVERYTHING on an instance level
        location = baseAddr(modloader_get_libil2cpp_path());
    }
    return location + (uintptr_t)offset;
}

int mkpath(std::string_view file_path) {
    return system((std::string("mkdir -p -m +rw ") + file_path.data()).c_str());
}

uintptr_t findPattern(uintptr_t dwAddress, const char* pattern, uintptr_t dwSearchRangeLen) {
#define in_range(x, a, b) (x >= a && x <= b)
#define get_bits(x) (in_range((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA) : (in_range(x, '0', '9') ? x - '0' : 0))
#define get_byte(x) (get_bits(x[0]) << 4 | get_bits(x[1]))

    // To avoid a lot of bad match candidates, pre-process wildcards at the front of the pattern
    uintptr_t skippedStartBytes = 0;
    while (pattern[0] == '\?') {
        // see comments below for insight on these numbers
        pattern += (pattern[1] == '\?') ? 3 : 2;
        skippedStartBytes++;
    }
    uintptr_t match = 0;  // current match candidate
    uintptr_t len = strlen(CRASH_UNLESS(pattern));
    if (dwSearchRangeLen < len) {
        return 0;
    }
    const char* pat = pattern;  // current spot in the pattern

    for (uintptr_t pCur = dwAddress + skippedStartBytes; pCur < dwAddress + dwSearchRangeLen; pCur++) {
        // If pat[0] is null char, we are done, or if pat >= pattern + len
        if (pat >= pattern + len || !pat[0] || !pat[1]) {
            return match;
        }
        // For each byte, if the pattern starts with a ? or the current byte matches:
        if (pat[0] == '\?' || *(char*)pCur == get_byte(pat)) {
            // If we do not have a match, begin it
            if (!match) {
                match = pCur - skippedStartBytes;
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
            if (match) pCur = match + skippedStartBytes;
            pat = pattern;
            match = 0;
        }
    }
    return 0;
}

uintptr_t findUniquePattern(bool& multiple, uintptr_t dwAddress, const char* pattern, const char* label, uintptr_t dwSearchRangeLen) {
    uintptr_t firstMatchAddr = 0, newMatchAddr, start = dwAddress, dwEnd = dwAddress + dwSearchRangeLen;
    int matches = 0;
    il2cpp_utils::Logger.debug("Sigscan for pattern: {}", pattern);
    while (start > 0 && start < dwEnd && (newMatchAddr = findPattern(start, pattern, dwEnd - start))) {
        if (!firstMatchAddr) firstMatchAddr = newMatchAddr;
        matches++;
        if (label) il2cpp_utils::Logger.debug("Sigscan found possible \"{}\": offset 0x{:x}, pointer 0x{:x}", label, newMatchAddr - dwAddress, newMatchAddr);
        start = newMatchAddr + 1;
        il2cpp_utils::Logger.debug("start = 0x{:x}, end = 0x{:x}", start, dwEnd);
        usleep(1000);
    }
    if (matches > 1) {
        multiple = true;
        il2cpp_utils::Logger.warn("Multiple sig scan matches for \"{}\"!", label);
    }
    return firstMatchAddr;
}

uintptr_t findUniquePatternInMappedFile(bool& multiple, const char* pattern, const char* file, const char* label) {
    // Essentially call findUniquePattern for each segment listed in /proc/self/maps
    std::ifstream procMap("/proc/self/maps");
    std::string line;
    uintptr_t firstMatchAddr = 0;
    while (std::getline(procMap, line)) {
        if (line.find(file) == std::string::npos) {
            continue;
        }
        auto idx = line.find_first_of('-');
        if (idx == std::string::npos) {
            CRASH_UNLESS(false);
        }
        auto startAddr = std::stoul(line.substr(0, idx), nullptr, 16);
        auto spaceIdx = line.find_first_of(' ');
        if (spaceIdx == std::string::npos) {
            CRASH_UNLESS(false);
        }
        auto endAddr = std::stoul(line.substr(idx + 1, spaceIdx - idx - 1), nullptr, 16);
        // Permissions are 4 characters
        auto perms = line.substr(spaceIdx + 1, 4);
        if (perms.find('r') != std::string::npos) {
            // Search between start and end
            uintptr_t match = findUniquePattern(multiple, startAddr, pattern, label, endAddr - startAddr);
            if(!firstMatchAddr)
                firstMatchAddr = match;
        }
    }
    procMap.close();
    return firstMatchAddr;
}

uintptr_t findUniquePatternInLibil2cpp(bool& multiple, const char* pattern, const char* label) {
   return findUniquePatternInMappedFile(multiple, pattern, "libil2cpp.so", label);
}

uintptr_t findUniquePatternInLibunity(bool& multiple, const char* pattern, const char* label) {
    return findUniquePatternInMappedFile(multiple, pattern, "libunity.so", label);
}

// C# SPECIFIC

void setcsstr(Il2CppString* in, std::u16string_view str) {
    in->length = str.length();
    for (int i = 0; i < in->length; i++) {
        // Can assume that each char is only a single char (a single word --> double word)
        in->chars[i] = str[i];
    }
    in->chars[in->length] = (Il2CppChar)'\u0000';
}


// Thanks DaNike!
void dump(int before, int after, void* ptr) {
    il2cpp_utils::Logger.debug("Dumping Immediate Pointer: {}: {:08x}", fmt::ptr(ptr), *reinterpret_cast<int*>(ptr));
    auto begin = static_cast<int*>(ptr) - before;
    auto end = static_cast<int*>(ptr) + after;
    for (auto cur = begin; cur != end; ++cur) {
        il2cpp_utils::Logger.debug("{}: {:08x}", fmt::ptr(cur), *cur);
    }
}

// SETTINGS

bool fileexists(std::string_view filename) {
    return access(filename.data(), W_OK | R_OK) != -1;
}

bool direxists(std::string_view dirname) {
    struct stat info;

    if (stat(dirname.data(), &info) != 0) {
        return false;
    }
    if (info.st_mode & S_IFDIR) {
        return true;
    }
    return false;
}

std::string readfile(std::string_view filename) {
    std::ifstream t(filename.data());
    if (!t.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

std::vector<char> readbytes(std::string_view filename) {
    std::ifstream infile(filename.data(), std::ios_base::binary);
    if (!infile.is_open()) {
        return std::vector<char>();
    }
    return std::vector<char>(std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>());
}

bool writefile(std::string_view filename, std::string_view text) {
    std::ofstream t(filename.data());
    if (t.is_open()) {
        t << text;
        return true;
    }
    return false;
}

bool deletefile(std::string_view filename) {
    if (fileexists(filename)) return remove(filename.data()) == 0;
    return false;
}

std::optional<std::string> getBuildId(std::string_view filename) {
    std::ifstream infile(filename.data(), std::ios_base::binary);
    if (!infile.is_open()) {
        return std::nullopt;
    }
    infile.seekg(0);
    ElfW(Ehdr) elf;
    infile.read(reinterpret_cast<char*>(&elf), sizeof(ElfW(Ehdr)));
    for(int i = 0; i < elf.e_shnum; i++) {
        infile.seekg(elf.e_shoff + i * elf.e_shentsize);
        ElfW(Shdr) section;
        infile.read(reinterpret_cast<char*>(&section), sizeof(ElfW(Shdr)));
        if(section.sh_type == SHT_NOTE && section.sh_size == 0x24) {
            char data[0x24];
            infile.seekg(section.sh_offset);
            infile.read(data, 0x24);
            ElfW(Nhdr)* note = reinterpret_cast<ElfW(Nhdr)*>(data);
            if(note->n_namesz == 4 && note->n_descsz == 20) {
                if(memcmp(reinterpret_cast<void*>(data + 12), "GNU", 4) == 0) {
                    std::stringstream stream;
                    stream << std::hex << std::setw(sizeof(uint8_t)*2);
                    auto buildIdAddr = reinterpret_cast<uint8_t*>(data + 16);
                    for(int i = 0; i < 5; i++) {
                        uint32_t value;
                        auto ptr = (reinterpret_cast<uint8_t*>(&value));
                        ptr[0] = *(buildIdAddr + i * sizeof(uint32_t) + 3);
                        ptr[1] = *(buildIdAddr + i * sizeof(uint32_t) + 2);
                        ptr[2] = *(buildIdAddr + i * sizeof(uint32_t) + 1);
                        ptr[3] = *(buildIdAddr + i * sizeof(uint32_t));
                        stream << value;
                    }
                    return stream.str();
                }
            }
        }
    }
    return std::nullopt;
}