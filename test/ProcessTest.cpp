#include "gtest/gtest.h"

#include <process.hpp>
#include <signal.h>
#include <sys/types.h>

#include <gmock/gmock.h>

#include <error.hpp>

namespace sdb::test {

    bool process_exists(pid_t aPid) {
        auto myRet = kill(aPid, 0);
        return myRet != -1 and errno != ESRCH;
    }

    TEST(ProcessTest, ProcessExistsAfterLaunch) {
        auto myProcess = Process::launch("yes");
        EXPECT_TRUE(process_exists(myProcess->get_pid()));
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

} // namespace sdb::test
