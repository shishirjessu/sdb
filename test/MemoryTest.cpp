#include "BreakpointSetTestHelpers.hpp"

#include "gtest/gtest.h"

#include <TestUtil.hpp>
#include <fmt/format.h>
#include <memory_operations.hpp>
#include <pipe.hpp>
#include <process.hpp>

namespace sdb::test {
    TEST(MemoryTest, TestMemoryOperations) {
        // Test reading
        Pipe myPipe{false};
        auto myProc =
            Process::launch("test/targets/memory", true, myPipe.getWrite());
        myPipe.closeWrite();

        myProc->resume();
        myProc->waitOnSignal();

        auto myPtr = fromBytes<std::uint64_t>(myPipe.read().data());
        auto myMemory = readMemory(myProc->getPid(), VirtualAddress{myPtr}, 8);
        auto myData = fromBytes<std::uint64_t>(myMemory.data());

        EXPECT_EQ(myData, 0xcafecafe);

        // Test writing
        myProc->resume();
        myProc->waitOnSignal();

        myPtr = fromBytes<std::uint64_t>(myPipe.read().data());

        auto mySpan = std::span<const std::byte>{asBytes("Hello, sdb!"), 12};
        writeMemory(myProc->getPid(), VirtualAddress{myPtr}, mySpan);

        myProc->resume();
        myProc->waitOnSignal();

        EXPECT_EQ(toStringView(myPipe.read()), "Hello, sdb!");
    };

} // namespace sdb::test
