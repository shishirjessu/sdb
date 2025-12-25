#include "gtest/gtest.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <pipe.hpp>
#include <process.hpp>
#include <signal.h>
#include <sys/types.h>
#include <variant>

#include <gmock/gmock.h>

#include <error.hpp>

namespace sdb::test {

    namespace {
        inline std::string_view toStringView(const std::byte* data,
                                             std::size_t size) {
            return {reinterpret_cast<const char*>(data), size};
        }

        std::string_view toStringView(const std::vector<std::byte>& aByteVec) {
            return toStringView(aByteVec.data(), aByteVec.size());
        }
    } // namespace

    TEST(RegisterTest, GeneralPurposeRegisterWrite) {
        Pipe myPipe{false};

        auto myProc =
            Process::launch("test/targets/reg_write", true, myPipe.getWrite());

        myPipe.closeWrite();

        myProc->resume();
        myProc->waitOnSignal();

        auto& myRegisters = myProc->getRegisters();

        myRegisters.write(findRegisterById(RegisterId::rsi), 0xcafecafe);

        // test the value we write to the local user struct can be obtained
        RegisterValueT myValue =
            myRegisters.read(findRegisterById(RegisterId::rsi));
        auto myVal = std::get<uint64_t>(myValue);
        EXPECT_EQ(myVal, 0xcafecafe);

        myProc->resume();
        myProc->waitOnSignal();

        auto output = myPipe.read();
        EXPECT_EQ(toStringView(output), "0xcafecafe");
    }
} // namespace sdb::test
