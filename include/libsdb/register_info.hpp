#pragma once

#include <cstdint>
#include <error.hpp>
#include <string_view>
#include <sys/user.h>

namespace sdb {
    enum class RegisterId {
        // general purpose registers
        rax,
        rdx,
        rcx,
        rbx,
        rsi,
        rdi,
        rbp,
        rsp,
        r8,
        r9,
        r10,
        r11,
        r12,
        r13,
        r14,
        r15,

        // instruction ptr and flags
        rip,
        eflags,

        // segment registers (mostly obsolete on x64)
        cs,
        fs,
        gs,
        ss,
        ds,
        es,

        // "fake register" used for syscall tracing
        orig_rax,

        // lower 32 bits
        eax,
        edx,
        ecx,
        ebx,
        esi,
        edi,
        ebp,
        esp,
        r8d,
        r9d,
        r10d,
        r11d,
        r12d,
        r13d,
        r14d,
        r15d,

        // lower 16 bits
        ax,
        dx,
        cx,
        bx,
        si,
        di,
        bp,
        sp,
        r8w,
        r9w,
        r10w,
        r11w,
        r12w,
        r13w,
        r14w,
        r15w,

        // high 8 bits
        ah,
        dh,
        ch,
        bh,

        // low 8 bits
        al,
        dl,
        cl,
        bl,
        sil,
        dil,
        bpl,
        spl,
        r8b,
        r9b,
        r10b,
        r11b,
        r12b,
        r13b,
        r14b,
        r15b,

        // floating point registers
        fcw,
        fsw,
        ftw,
        fop,
        frip,
        frdp,
        mxcsr,
        mxcsrmask,

        // 80 bit floating point regs
        st0,
        st1,
        st2,
        st3,
        st4,
        st5,
        st6,
        st7,

        // MMX (old style SIMD registers)
        // map to the same memory as the floating point regs above!
        mm0,
        mm1,
        mm2,
        mm3,
        mm4,
        mm5,
        mm6,
        mm7,

        // modern 128 bit SIMD registers
        xmm0,
        xmm1,
        xmm2,
        xmm3,
        xmm4,
        xmm5,
        xmm6,
        xmm7,
        xmm8,
        xmm9,
        xmm10,
        xmm11,
        xmm12,
        xmm13,
        xmm14,
        xmm15,

        // debug registers
        dr0,
        dr1,
        dr2,
        dr3,
        dr4,
        dr5,
        dr6,
        dr7
    };

    enum class RegisterType { gpr, sub_gpr, fpr, dr };

    enum class RegisterFormat { UInt, DoubleFloat, LongDouble, Vector };

    struct RegisterInfo {
        RegisterId theId;
        std::string_view theName;
        std::int32_t theDwarfId;
        std::size_t theSize;
        std::size_t theOffset;
        RegisterType theRegisterType;
        RegisterFormat theRegisterFormat;
    };

    inline constexpr const RegisterInfo g_register_infos[] = {
#define DEFINE_REGISTER(name, dwarf_id, size, offset, type, format)            \
    { RegisterId::name, #name, dwarf_id, size, offset, type, format }
#include <libsdb/detail/registers.inc>
#undef DEFINE_REGISTER
    };

    template <typename PredicateT>
    const RegisterInfo& findRegisterByPredicate(PredicateT aPredicate) {
        auto myIt = std::ranges::find_if(g_register_infos, aPredicate);
        if (myIt == std::end(g_register_infos)) {
            Error::send("Can't find register info");
        }

        return *myIt;
    }

    inline const RegisterInfo& findRegisterById(RegisterId anId) {
        return findRegisterByPredicate(
            [anId](const RegisterInfo& aRegisterInfo) {
                return aRegisterInfo.theId == anId;
            });
    }

    inline const RegisterInfo& findRegisterByName(const std::string& aName) {
        return findRegisterByPredicate(
            [&aName](const RegisterInfo& aRegisterInfo) {
                return aRegisterInfo.theName == aName;
            });
    }

    inline const RegisterInfo& findRegisterByDwarfId(std::int32_t aDwarfId) {
        return findRegisterByPredicate(
            [&aDwarfId](const RegisterInfo& aRegisterInfo) {
                return aRegisterInfo.theDwarfId == aDwarfId;
            });
    }

} // namespace sdb
