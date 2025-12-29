#include "BreakpointSetTestHelpers.hpp"

#include "gtest/gtest.h"

#include <TestUtil.hpp>
#include <fmt/format.h>
#include <pipe.hpp>
#include <process.hpp>

namespace sdb::test {

    TEST(BreakpointSetTest, SetBreakpoint) {
        Pipe myPipe(false);

        auto myProcess =
            Process::launch("test/targets/hello_sdb", true, myPipe.getWrite());

        myPipe.closeWrite();

        VirtualAddress myLoadAddress =
            get_load_address(myProcess->getPid(),
                             get_entry_point_offset("test/targets/hello_sdb"));

        auto& myBp = myProcess->createBreakpointSite(myLoadAddress);
        myBp.enable();

        myProcess->resume();
        auto myReason = myProcess->waitOnSignal();

        EXPECT_EQ(myReason.theStopState, ProcessState::Stopped);
        EXPECT_EQ(myReason.theStatus, SIGTRAP);
        EXPECT_EQ(myProcess->getPc(), myLoadAddress);

        myProcess->resume();
        myReason = myProcess->waitOnSignal();

        EXPECT_EQ(myReason, StopReason{0});

        auto data = myPipe.read();
        EXPECT_EQ(toStringView(data), "Hello, sdb!\n");
    }

    TEST(BreakpointTest, RemoveBreakpoint) {
        auto myProcess = Process::launch("test/targets/hello_sdb", true);

        auto& myBp1 = myProcess->createBreakpointSite(VirtualAddress{1});
        auto& myBp2 = myProcess->createBreakpointSite(VirtualAddress{2});

        auto& myBreakpointSites = myProcess->getBreakpointSites();
        EXPECT_EQ(myBreakpointSites.size(), 2);

        myProcess->getBreakpointSites().removeByAddress(myBp1.getAddress());
        EXPECT_EQ(myBreakpointSites.size(), 1);

        myProcess->getBreakpointSites().removeById(myBp2.getId());
        EXPECT_EQ(myBreakpointSites.size(), 0);
    }

} // namespace sdb::test
