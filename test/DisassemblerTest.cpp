#include <BreakpointSetTestHelpers.hpp>

#include "gtest/gtest.h"
#include <gmock/gmock.h>

#include <TestUtil.hpp>
#include <disassembler.hpp>
#include <memory_operations.hpp>
#include <process.hpp>

namespace sdb::test {
    TEST(DisassemblerTest, TestDisassembly) {
        auto myProcess = Process::launch("test/targets/hello_sdb", true);

        VirtualAddress myLoadAddress =
            get_load_address(myProcess->getPid(),
                             get_entry_point_offset("test/targets/hello_sdb"));

        auto& myBp = myProcess->createBreakpointSite(myLoadAddress);
        myBp.enable();

        myProcess->resume();
        myProcess->waitOnSignal();

        Disassembler myDisassembler{*myProcess};

        std::vector<Instruction> myInstructions = myDisassembler.disassemble(5);
        EXPECT_THAT(
            myInstructions,
            ::testing::ElementsAre(
                Instruction{VirtualAddress{0x0000555555555050}, "endbr64 "},
                Instruction{VirtualAddress{0x0000555555555054},
                            "xor    %ebp,%ebp"},
                Instruction{VirtualAddress{0x0000555555555056},
                            "mov    %rdx,%r9"},
                Instruction{VirtualAddress{0x0000555555555059}, "pop    %rsi"},
                Instruction{VirtualAddress{0x000055555555505a},
                            "mov    %rsp,%rdx"}));
    };

} // namespace sdb::test
