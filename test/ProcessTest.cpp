#include "gtest/gtest.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <process.hpp>
#include <signal.h>
#include <sys/types.h>

#include <gmock/gmock.h>

#include <error.hpp>

namespace sdb::test {

    namespace {
        bool processExists(pid_t aPid) {
            auto myRet = kill(aPid, 0);
            return myRet != -1 and errno != ESRCH;
        }

        char processStatus(pid_t aPid) {
            std::filesystem::path myStatFile{
                std::format("/proc/{}/stat", aPid)};

            std::ifstream myStream(myStatFile);
            EXPECT_TRUE(myStream.good());

            std::string myFirstLine;
            std::getline(myStream, myFirstLine);

            std::cout << myFirstLine << '\n';

            auto myLastParenIdx = myFirstLine.rfind(')');
            return myFirstLine[myLastParenIdx + 2];
        }
    } // namespace

    TEST(ProcessTest, ProcessExistsAfterLaunch) {
        auto myProcess = Process::launch("yes");
        EXPECT_TRUE(processExists(myProcess->getPid()));
    }

    TEST(ProcessTest, ProcessLaunchThrowsIfFileDoesNotExist) {
        EXPECT_THROW(
            {
                try {
                    Process::launch("fake");
                } catch (const sdb::Error& e) {
                    EXPECT_THAT(e.what(), ::testing::HasSubstr("exec failed"));
                    throw;
                }
            },
            sdb::Error);
    }

    TEST(ProcessTest, ProcessStatusAfterAttachIsTracingStop) {
        static constexpr char TRACING_STOP{'t'};
        static constexpr bool NO_DEBUG{false};

        auto myProcessUnderTest = Process::launch("yes", NO_DEBUG);
        auto myAttachedProc = Process::attach(myProcessUnderTest->getPid());

        EXPECT_EQ(processStatus(myAttachedProc->getPid()), TRACING_STOP);
    }

    TEST(ProcessTest, FailsToAttachToPidZero) {
        EXPECT_THROW(
            {
                try {
                    Process::attach(0);
                } catch (const sdb::Error& e) {
                    EXPECT_THAT(e.what(), ::testing::HasSubstr("invalid pid"));
                    throw;
                }
            },
            sdb::Error);
    }

    TEST(ProcessTest, ResumingFailsIfProcessEnded) {
        auto myProc = Process::launch("true");

        myProc->resume();
        myProc->waitOnSignal();

        EXPECT_THROW(
            {
                try {
                    myProc->resume();
                } catch (const sdb::Error& e) {
                    EXPECT_THAT(e.what(),
                                ::testing::HasSubstr("resume failed"));
                    throw;
                }
            },
            sdb::Error);
    }

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

} // namespace sdb::test
