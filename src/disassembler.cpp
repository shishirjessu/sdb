#include <disassembler.hpp>
#include <memory_operations.hpp>
#include <process.hpp>

#include <iostream>
#include <vector>

#include <dis-asm.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

namespace sdb {
    namespace {
        // Copied from
        // https://blog.yossarian.net/2019/05/18/Basic-disassembly-with-libopcodes
        static int dis_fprintf(void* stream, const char* fmt, ...) {
            stream_state* ss = (stream_state*)stream;

            va_list arg;
            va_start(arg, fmt);
            if (!ss->reenter) {
                vasprintf(&ss->insn_buffer, fmt, arg);
                ss->reenter = true;
            } else {
                char* tmp;
                vasprintf(&tmp, fmt, arg);

                char* tmp2;
                asprintf(&tmp2, "%s%s", ss->insn_buffer, tmp);
                free(ss->insn_buffer);
                free(tmp);
                ss->insn_buffer = tmp2;
            }
            va_end(arg);

            return 0;
        }
    } // namespace

    Disassembler::Disassembler(Process& aProcess) : theProcess{aProcess} {
        init_disassemble_info(&disasm_info, &ss, dis_fprintf);
        disasm_info.arch = bfd_arch_i386;
        disasm_info.mach = bfd_mach_x86_64;
        disasm_info.read_memory_func = buffer_read_memory;
        disasm_info.buffer_vma = 0;
        disassemble_init_for_target(&disasm_info);

        disasm = disassembler(bfd_arch_i386, false, bfd_mach_x86_64, NULL);
    }

    std::vector<Instruction>
    Disassembler::disassemble(std::size_t aNumInstructions,
                              std::optional<VirtualAddress> aStartingAddress) {
        VirtualAddress myAddr = aStartingAddress.value_or(theProcess.getPc());

        std::vector<Instruction> myResult;
        myResult.reserve(aNumInstructions);

        auto myMemory = readMemoryWithoutBreakpointTraps(
            theProcess, myAddr, aNumInstructions * X64_MAX_INSTR_SIZE);

        auto myIntegralInputBuffer = convertToIntegral(myMemory);

        disasm_info.buffer = myIntegralInputBuffer.data();
        disasm_info.buffer_length = myIntegralInputBuffer.size();
        disassemble_init_for_target(&disasm_info);

        size_t myCurPos = 0;
        while (aNumInstructions-- > 0) {
            size_t myInstrSize = disasm(myCurPos, &disasm_info);

            myResult.emplace_back(myAddr + myCurPos,
                                  std::string{ss.insn_buffer});

            free(ss.insn_buffer);
            ss.reenter = false;

            myCurPos += myInstrSize;
        }

        return myResult;
    }

} // namespace sdb
