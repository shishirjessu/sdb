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

} // namespace sdb::test
