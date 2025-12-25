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
        // set up test and run child proc until first trap
        Pipe myPipe{false};

        auto myProc =
            Process::launch("test/targets/reg_write", true, myPipe.getWrite());

        myPipe.closeWrite();

        myProc->resume();
        myProc->waitOnSignal();

        // write rsi and resume til 2nd trap
        auto& myRegisters = myProc->getRegisters();

        myRegisters.write(findRegisterById(RegisterId::rsi), 0xcafecafe);

        myProc->resume();
        myProc->waitOnSignal();

        auto output = myPipe.read();
        EXPECT_EQ(toStringView(output), "0xcafecafe");

        // write mm0 and resume til 3rd trap
        myRegisters.write(findRegisterById(RegisterId::mm0), 0xabcdef);

        myProc->resume();
        myProc->waitOnSignal();

        output = myPipe.read();
        EXPECT_EQ(toStringView(output), "0xabcdef");

        // write xmm0 and resume til 4th trap
        myRegisters.write(findRegisterById(RegisterId::xmm0), 67.21);
        myProc->resume();
        myProc->waitOnSignal();

        output = myPipe.read();
        EXPECT_EQ(toStringView(output), "67.21");

        // write st0 and resume til last trap
        myRegisters.write(findRegisterById(RegisterId::st0), 67.21l);
        myRegisters.write(findRegisterById(RegisterId::fsw),
                          std::uint16_t{0b0011100000000000});
        myRegisters.write(findRegisterById(RegisterId::ftw),
                          std::uint16_t{0b0011111111111111});

        myProc->resume();
        myProc->waitOnSignal();

        output = myPipe.read();
        EXPECT_EQ(toStringView(output), "67.21");
    }

} // namespace sdb::test
