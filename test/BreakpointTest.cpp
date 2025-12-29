#include "BreakpointSetTestHelpers.hpp"

#include "gtest/gtest.h"

#include <TestUtil.hpp>
#include <fmt/format.h>
#include <pipe.hpp>
#include <process.hpp>

namespace sdb::test {
    TEST(ProcessTest, CreateBreakpointSites) {
        auto myProc = Process::launch("test/targets/run_forever");

        const auto& myBreakpointSites = myProc->getBreakpointSites();
        EXPECT_EQ(myBreakpointSites.size(), 0);
        EXPECT_TRUE(myBreakpointSites.empty());

        auto myBreakpointSiteCheck = [&](const BreakpointSite& mySite,
                                         VirtualAddress anAddress,
                                         BreakpointSiteId anId) {
            EXPECT_EQ(mySite.getId(), anId);
            EXPECT_EQ(mySite.getAddress(), anAddress);

            EXPECT_TRUE(myBreakpointSites.contains_id(anId));
            EXPECT_TRUE(myBreakpointSites.contains_address(anAddress));

            EXPECT_EQ(myBreakpointSites.getById(anId).getId(), anId);
            EXPECT_EQ(myBreakpointSites.getByAddress(anAddress).getAddress(),
                      anAddress);
        };

        BreakpointSite& myFirstSite =
            myProc->createBreakpointSite(VirtualAddress{0});
        EXPECT_EQ(myBreakpointSites.size(), 1);
        EXPECT_FALSE(myBreakpointSites.empty());

        EXPECT_EQ(myFirstSite.getAddress(), VirtualAddress{0});

        for (std::uint64_t i = 1; i < 10; ++i) {
            BreakpointSite& myCurSite =
                myProc->createBreakpointSite(VirtualAddress{i});

            myBreakpointSiteCheck(myCurSite, VirtualAddress{i},
                                  myFirstSite.getId() + i);

            EXPECT_EQ(myBreakpointSites.size(), i + 1);
            EXPECT_FALSE(myBreakpointSites.empty());
        }
    }

    TEST(ProcessTest, MissingBreakpointSites) {
        auto myProc = Process::launch("test/targets/run_forever");
        const auto& cproc = myProc;

        EXPECT_THROW(
            myProc->getBreakpointSites().getByAddress(VirtualAddress{44}),
            sdb::Error);
        EXPECT_THROW(myProc->getBreakpointSites().getById(BreakpointSiteId{44}),
                     sdb::Error);
        EXPECT_THROW(
            cproc->getBreakpointSites().getByAddress(VirtualAddress{44}),
            sdb::Error);
        EXPECT_THROW(cproc->getBreakpointSites().getById(BreakpointSiteId{44}),
                     sdb::Error);
    }

    TEST(ProcessTest, IteratingOverBreakpoints) {
        auto myProc = Process::launch("test/targets/run_forever");
        const auto& cproc = myProc;

        VirtualAddress myStartingAddr{42};

        myProc->createBreakpointSite(myStartingAddr);
        myProc->createBreakpointSite(myStartingAddr + 1);
        myProc->createBreakpointSite(myStartingAddr + 2);
        myProc->createBreakpointSite(myStartingAddr + 3);

        myProc->getBreakpointSites().forEach(
            [myStartingAddr](auto& mySite) mutable {
                EXPECT_EQ(mySite.getAddress(), myStartingAddr++);
            });

        cproc->getBreakpointSites().forEach(
            [myStartingAddr](auto& mySite) mutable {
                EXPECT_EQ(mySite.getAddress(), myStartingAddr++);
            });
    }

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
