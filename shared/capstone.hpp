#pragma once

#include "config.hpp"
#include "capstone/shared/capstone/capstone.h"
#include "capstone/shared/platform.h"
#include "flamingo/shared/installer.hpp"
#include "flamingo/shared/target-data.hpp"

namespace cs {
    csh get_handle();

    uint32_t* readb(uint32_t const* addr);

    template <arm64_insn... Args>
    constexpr bool insn_match(cs_insn* insn) {
        if constexpr (sizeof...(Args) > 0) {
            return (((insn->id == Args) || ...));
        }
        return false;
    };

    struct AddrSearchPair {
        AddrSearchPair(uint32_t const* addr, uint32_t rem_search_size) : addr(addr), rem_search_size(rem_search_size) {}
        uint32_t const* addr;
        uint64_t rem_search_size;
    };

    auto find_through_hooks(void const* hook, uint32_t init_search_size, auto&& func) {
        // First, check to see if we are hooked.
        i2c::logger.debug("Finding through potential hook: {} and size: {}", fmt::ptr(hook), init_search_size);
        auto original_insts = flamingo::OriginalInstsFor(flamingo::TargetDescriptor{const_cast<void*>(hook)});
        // If flamingo reported there was a hook here, it'll give us a span over the original instructions.
        if (original_insts.size() > 0) {
            i2c::logger.debug("Found original instructions of length: {}", original_insts.size());
            if (init_search_size < original_insts.size() * sizeof(uint32_t)) {
                return func(cs::AddrSearchPair(original_insts.data(), init_search_size));
            }
            return func(
                cs::AddrSearchPair(original_insts.data(), original_insts.size() * sizeof(uint32_t)),
                cs::AddrSearchPair(
                    reinterpret_cast<uint32_t const*>(hook) + original_insts.size(), init_search_size - original_insts.size() * sizeof(uint32_t)
                )
            );
        }
        i2c::logger.debug("No hook found! Searching: {}, {}", fmt::ptr(hook), init_search_size);
        return func(cs::AddrSearchPair(reinterpret_cast<uint32_t const*>(hook), init_search_size));
    }

    template <std::size_t Sz, typename F1, typename F2>
    auto find_nth(std::array<AddrSearchPair, Sz>& addrs, uint32_t n_to_ret_on, int ret_count, F1&& match, F2&& skip) {
        cs_insn* insn = cs_malloc(get_handle());
        for (std::size_t search_idx = 0; search_idx < addrs.size(); search_idx++) {
            while (addrs[search_idx].rem_search_size > 0) {
                auto ptr = reinterpret_cast<uint64_t>(addrs[search_idx].addr);
                bool res = cs_disasm_iter(
                    get_handle(), reinterpret_cast<uint8_t const**>(&addrs[search_idx].addr), &addrs[search_idx].rem_search_size, &ptr, insn
                );
                i2c::logger.debug(
                    "{} diassemb: {} (r_count: {}, n_to_ret_on: {}, sz: {})",
                    fmt::ptr((void*) ptr),
                    insn->mnemonic,
                    ret_count,
                    n_to_ret_on,
                    addrs[search_idx].rem_search_size
                );
                if (res) {
                    // Valid decode, so lets check to see if it is a match or we need to break.
                    if (insn->id == ARM64_INS_RET) {
                        if (ret_count == 0) {
                            // Early termination!
                            cs_free(insn, 1);
                            i2c::logger.warn(
                                "Could not find: {} call at: {} within: {} rets! Found all of the rets first!",
                                n_to_ret_on,
                                fmt::ptr(addrs[search_idx].addr),
                                ret_count
                            );
                            return (decltype(match(insn))) std::nullopt;
                        }
                        ret_count--;
                    } else {
                        auto testRes = match(insn);
                        if (testRes) {
                            if (n_to_ret_on == 1) {
                                cs_free(insn, 1);
                                return testRes;
                            } else {
                                n_to_ret_on--;
                            }
                        } else if (skip(insn)) {
                            if (n_to_ret_on == 1) {
                                std::string name(insn->mnemonic);
                                cs_free(insn, 1);
                                i2c::logger.warn(
                                    "Found: {} match, at: {} within: {} rets, but the result was a {}! Cannot compute destination address!",
                                    n_to_ret_on,
                                    fmt::ptr(addrs[search_idx].addr),
                                    ret_count,
                                    name
                                );
                                return (decltype(match(insn))) std::nullopt;
                            } else {
                                n_to_ret_on--;
                            }
                        }
                    }
                    // Other instructions are ignored silently
                } else {
                    // Invalid instructions are ignored silently.
                    // In order to skip these properly, we must increment our instructions, ptr, and size accordingly.
                    addrs[search_idx].rem_search_size -= 4;
                    addrs[search_idx].addr++;
                }
            }
            // We didn't find it. Let's instead look at the next address/size pair for a match.
            i2c::logger.debug(
                "Could not find: {} call at: {} within: {} rets at idx: {}!", n_to_ret_on, fmt::ptr(addrs[search_idx].addr), ret_count, search_idx
            );
        }
        // If we run out of bytes to parse, we fail
        cs_free(insn, 1);
        return (decltype(match(insn))) std::nullopt;
    }

    template <uint32_t NToRetOn, int RetCount = -1, size_t SzBytes = 4096, typename F1, typename F2>
    requires((NToRetOn >= 1 && (SzBytes % 4) == 0))
    auto find_nth(uint32_t const* addr, F1&& match, F2&& skip) {
        cs_insn* insn = cs_malloc(get_handle());
        auto ptr = reinterpret_cast<uint64_t>(addr);
        auto instructions = reinterpret_cast<uint8_t const*>(addr);

        int curr_r_count = RetCount;
        uint32_t n_calls = NToRetOn;
        size_t curr_sz = SzBytes;
        while (curr_sz > 0) {
            bool res = cs_disasm_iter(get_handle(), &instructions, &curr_sz, &ptr, insn);
            i2c::logger.debug(
                "{} diassemb: {} (r_count: {}, n_calls: {}, sz: {})", fmt::ptr((void*) ptr), insn->mnemonic, curr_r_count, n_calls, curr_sz
            );
            if (res) {
                // Valid decode, so lets check to see if it is a match or we need to break.
                if (insn->id == ARM64_INS_RET) {
                    if (curr_r_count == 0) {
                        // Early termination!
                        cs_free(insn, 1);
                        i2c::logger.warn(
                            "Could not find: {} call at: {} within: {} rets! Found all of the rets first!", NToRetOn, fmt::ptr((void*) ptr), RetCount
                        );
                        return (decltype(match(insn))) std::nullopt;
                    }
                    curr_r_count--;
                } else {
                    if (auto test_res = match(insn)) {
                        if (n_calls == 1) {
                            cs_free(insn, 1);
                            return test_res;
                        } else {
                            n_calls--;
                        }
                    } else if (skip(insn)) {
                        if (n_calls == 1) {
                            std::string name(insn->mnemonic);
                            cs_free(insn, 1);
                            i2c::logger.warn(
                                "Found: {} match, at: {} within: {} rets, but the result was a {}! Cannot compute destination address!",
                                NToRetOn,
                                fmt::ptr((void*) ptr),
                                RetCount,
                                name
                            );
                            return (decltype(match(insn))) std::nullopt;
                        } else {
                            n_calls--;
                        }
                    }
                }
                // Other instructions are ignored silently
            } else {
                // Invalid instructions are ignored silently.
                // In order to skip these properly, we must increment our instructions, ptr, and size accordingly.
                curr_sz -= 4;
                ptr += 4;
                instructions += 4;
            }
        }
        // If we run out of bytes to parse, we fail
        cs_free(insn, 1);
        i2c::logger.warn("Could not find: {} call at: {} within: {} rets, within size: {}!", NToRetOn, fmt::ptr(addr), RetCount, SzBytes);
        return (decltype(match(insn))) std::nullopt;
    }

    template <uint32_t nToRetOn, auto match, auto skip, int retCount = -1, size_t szBytes = 4096>
    requires((nToRetOn >= 1 && (szBytes % 4) == 0))
    auto find_nth(uint32_t const* addr) {
        cs_insn* insn = cs_malloc(get_handle());
        auto ptr = reinterpret_cast<uint64_t>(addr);
        auto instructions = reinterpret_cast<uint8_t const*>(addr);

        int curr_r_count = retCount;
        uint32_t n_calls = nToRetOn;
        size_t curr_sz = szBytes;
        while (curr_sz > 0) {
            bool res = cs_disasm_iter(get_handle(), &instructions, &curr_sz, &ptr, insn);
            i2c::logger.debug(
                "{} diassemb: {} (r_count: {}, n_calls: {}, sz: {})", fmt::ptr((void*) ptr), insn->mnemonic, curr_r_count, n_calls, curr_sz
            );
            if (res) {
                // Valid decode, so lets check to see if it is a match or we need to break.
                if (insn->id == ARM64_INS_RET) {
                    if (curr_r_count == 0) {
                        // Early termination!
                        cs_free(insn, 1);
                        i2c::logger.warn(
                            "Could not find: {} call at: {} within: {} rets! Found all of the rets first!", nToRetOn, fmt::ptr((void*) ptr), retCount
                        );
                        return (decltype(match(insn))) std::nullopt;
                    }
                    curr_r_count--;
                } else {
                    if (auto test_res = match(insn)) {
                        if (n_calls == 1) {
                            cs_free(insn, 1);
                            return test_res;
                        } else {
                            n_calls--;
                        }
                    } else if (skip(insn)) {
                        if (n_calls == 1) {
                            std::string name(insn->mnemonic);
                            cs_free(insn, 1);
                            i2c::logger.warn(
                                "Found: {} match, at: {} within: {} rets, but the result was a {}! Cannot compute destination address!",
                                nToRetOn,
                                fmt::ptr((void*) ptr),
                                retCount,
                                name
                            );
                            return (decltype(match(insn))) std::nullopt;
                        } else {
                            n_calls--;
                        }
                    }
                }
                // Other instructions are ignored silently
            } else {
                // Invalid instructions are ignored silently.
                // In order to skip these properly, we must increment our instructions, ptr, and size accordingly.
                i2c::logger.warn("FAILED PARSE: {} diassemb: 0x{:x}", fmt::ptr((void*) ptr), *(uint32_t*) ptr);
                curr_sz -= 4;
                ptr += 4;
                instructions += 4;
            }
        }
        // If we run out of bytes to parse, we fail
        cs_free(insn, 1);
        return (decltype(match(insn))) std::nullopt;
    }

    std::optional<uint32_t*> bl_conv(cs_insn* insn);

    template <uint32_t NToRetOn, bool IncludeR = false, int RetCount = -1, size_t SzBytes = 4096>
    requires((NToRetOn >= 1 && (SzBytes % 4) == 0))
    auto find_nth_bl(uint32_t const* addr) {
        return find_through_hooks(addr, SzBytes, [](auto... pairs) {
            std::array addrs{pairs...};
            if constexpr (IncludeR) {
                return find_nth(addrs, NToRetOn, RetCount, &bl_conv, &insn_match<ARM64_INS_BLR>);
            } else {
                return find_nth(addrs, NToRetOn, RetCount, &bl_conv, &insn_match<>);
            }
        });
    }

    std::optional<uint32_t*> b_conv(cs_insn* insn);

    template <uint32_t NToRetOn, bool IncludeR = false, int RetCount = -1, size_t SzBytes = 4096>
    requires((NToRetOn >= 1 && (SzBytes % 4) == 0))
    auto find_nth_b(uint32_t const* addr) {
        return find_through_hooks(addr, SzBytes, [](auto... pairs) {
            std::array addrs{pairs...};
            if constexpr (IncludeR) {
                return find_nth(addrs, NToRetOn, RetCount, &b_conv, &insn_match<ARM64_INS_BR>);
            } else {
                return find_nth(addrs, NToRetOn, RetCount, &b_conv, &insn_match<>);
            }
        });
    }

    std::optional<std::tuple<uint32_t*, arm64_reg, uint32_t*>> pc_rel_conv(cs_insn* insn);

    template <uint32_t NToRetOn, int RetCount = -1, size_t SzBytes = 4096>
    requires((NToRetOn >= 1 && (SzBytes % 4) == 0))
    auto find_nth_pc_rel(uint32_t const* addr) {
        return find_through_hooks(addr, SzBytes, [](auto... pairs) {
            std::array addrs{pairs...};
            return find_nth(addrs, NToRetOn, RetCount, &pc_rel_conv, &insn_match<>);
        });
    }

    std::optional<std::tuple<uint32_t*, arm64_reg, int64_t>> reg_match_conv(cs_insn* match, arm64_reg to_match);

    template <uint32_t NToRetOn, int RetCount = -1, size_t SzBytes = 4096>
    requires((NToRetOn >= 1 && (SzBytes % 4) == 0))
    auto find_nth_reg(uint32_t const* addr, arm64_reg reg) {
        auto lmd = [reg](cs_insn* in) -> std::optional<std::tuple<uint32_t*, arm64_reg, int64_t>> {
            return reg_match_conv(in, reg);
        };
        return find_through_hooks(addr, SzBytes, [lmd = std::move(lmd)](auto... pairs) {
            std::array addrs{pairs...};
            return find_nth(addrs, NToRetOn, RetCount, lmd, &insn_match<>);
        });
    }

    template <uint32_t NToRetOn, uint32_t NImmOff, size_t SzBytes = 4096>
    requires((NToRetOn >= 1 && NImmOff >= 1 && (SzBytes % 4) == 0))
    std::optional<std::tuple<uint32_t*, arm64_reg, uint32_t*>> getpcaddr(uint32_t const* addr) {
        auto pcrel = find_nth_pc_rel<NToRetOn, -1, SzBytes>(addr);
        // SAFE_ABORT("Could not find: {} pcrel at: {p within: {} rets, within size: {}!", NToRetOn, addr, -1, SzBytes);
        if (!pcrel) {
            return std::nullopt;
        }
        // addr is in first slot of tuple, reg in second, dst imm in third
        // TODO: decrease size correctly
        auto reginst = find_nth_reg<NImmOff, -1, SzBytes>(std::get<0>(*pcrel), std::get<1>(*pcrel));
        // SAFE_ABORT("Could not find: {} reg with reg: {} at: {p within: {} rets, within size: {}!", NImmOff, std::get<1>(*pcrel),
        // std::get<0>(*pcrel), -1, szBytes);
        if (!reginst) {
            return std::nullopt;
        }
        return std::make_tuple(
            std::get<0>(*reginst),
            std::get<1>(*reginst),
            reinterpret_cast<uint32_t*>(reinterpret_cast<uint64_t>(std::get<2>(*pcrel)) + std::get<2>(*reginst))
        );
    }

    template <uint32_t NToRetOn, uint32_t NImmOff, int Match, size_t SzBytes = 4096>
    requires((NToRetOn >= 1 && NImmOff >= 1 && (SzBytes % 4) == 0))
    std::optional<uint32_t*> evalswitch(uint32_t const* addr) {
        // Get matching adr/adrp + offset on register
        auto res = getpcaddr<NToRetOn, NImmOff, SzBytes>(addr);
        // SAFE_ABORT("Could not find: {} pcrel at: {p within: {} rets, within size: {}!", NToRetOn, addr, -1, SzBytes);
        if (!res) {
            return std::nullopt;
        }
        // Convert destination to the switch table address
        auto switch_table = reinterpret_cast<int32_t*>(std::get<2>(*res));
        // Index into switch table, which holds int32s, offset from start of switch table
        auto val = switch_table[Match - 1];
        // Add offset to switch table and convert back to pointer type
        return reinterpret_cast<uint32_t*>(reinterpret_cast<uint64_t>(switch_table) + val);
    }
}
