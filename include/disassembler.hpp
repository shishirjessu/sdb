#pragma once

#define PRINT_INSN_FOR_I386

#include <bfd.h>
#include <dis-asm.h>

#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include <types.hpp>

namespace sdb {
    static constexpr std::size_t X64_MAX_INSTR_SIZE{15};

    struct stream_state {
        char* insn_buffer;
        bool reenter;
    };

    struct Instruction {
        VirtualAddress theAddress;
        std::string theInstruction;

        bool operator==(const Instruction& other) const = default;
    };

    inline std::ostream& operator<<(std::ostream& os,
                                    const Instruction& instr) {
        const auto flags = os.flags();

        os << "0x" << std::hex << std::to_underlying(instr.theAddress) << ": "
           << instr.theInstruction;

        os.flags(flags);
        return os;
    }

    class Process;

    class Disassembler {
      public:
        Disassembler(Process& aProcess);
        std::vector<Instruction> disassemble(
            std::size_t aNumInstructions,
            std::optional<VirtualAddress> aStartingAddress = std::nullopt);

      private:
        disassemble_info disasm_info{};
        disassembler_ftype disasm{};
        stream_state ss{};

        Process& theProcess;

        static std::vector<uint8_t>
        convertToIntegral(const std::vector<std::byte>& vec) {
            std::vector<uint8_t> res;
            for (auto v : vec) {
                res.push_back(static_cast<uint8_t>(v));
            }
            return res;
        }
    };

} // namespace sdb
