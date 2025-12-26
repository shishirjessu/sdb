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

        inline std::string_view toStringView(const std::byte* aData,
                                             std::size_t aSize) {
            return {reinterpret_cast<const char*>(aData), aSize};
        }

        inline std::string_view
        toStringView(const std::vector<std::byte>& aBytes) {
            return toStringView(aBytes.data(), aBytes.size());
        }

        inline void resumeToNextTrap(Process& aProc) {
            aProc.resume();
            aProc.waitOnSignal();
        }

        template <typename T>
        void writeAndExpect(Process& aProc, Pipe& aPipe, RegisterId aRegister,
                            T aValue, std::string_view aExpected) {
            aProc.getRegisters().write(findRegisterById(aRegister), aValue);
            resumeToNextTrap(aProc);

            auto myOutput = aPipe.read();
            EXPECT_EQ(toStringView(myOutput), aExpected);
        }

        void writeSt0AndExpect(Process& aProc, Pipe& aPipe, long double aValue,
                               std::string_view aExpected) {
            auto& myRegisters = aProc.getRegisters();

            myRegisters.write(findRegisterById(RegisterId::st0), aValue);
            myRegisters.write(findRegisterById(RegisterId::fsw),
                              std::uint16_t{0b0011100000000000});
            myRegisters.write(findRegisterById(RegisterId::ftw),
                              std::uint16_t{0b0011111111111111});

            resumeToNextTrap(aProc);

            auto myOutput = aPipe.read();
            EXPECT_EQ(toStringView(myOutput), aExpected);
        }

        template <typename T>
        void resumeAndExpectRead(Process& aProc, RegisterId aRegister,
                                 const T& aExpected) {
            resumeToNextTrap(aProc);

            EXPECT_EQ(std::get<T>(aProc.getRegisters().read(
                          findRegisterById(aRegister))),
                      aExpected);
        }

    } // namespace

    TEST(RegisterTest, WriteRegisters) {
        Pipe myPipe{false};

        auto myProc =
            Process::launch("test/targets/reg_write", true, myPipe.getWrite());

        myPipe.closeWrite();

        resumeToNextTrap(*myProc);

        writeAndExpect(*myProc, myPipe, RegisterId::rsi, 0xcafecafe,
                       "0xcafecafe");

        writeAndExpect(*myProc, myPipe, RegisterId::mm0, 0xabcdef, "0xabcdef");
        writeAndExpect(*myProc, myPipe, RegisterId::xmm0, 67.21, "67.21");

        writeSt0AndExpect(*myProc, myPipe, 67.21L, "67.21");
    }

    TEST(RegisterTest, ReadRegisters) {
        auto myProc = Process::launch("test/targets/reg_read", true);

        resumeAndExpectRead<uint64_t>(*myProc, RegisterId::r13, 0xcafecafe);
        resumeAndExpectRead<uint8_t>(*myProc, RegisterId::r13b, 49);

        resumeAndExpectRead<Byte64>(*myProc, RegisterId::mm0,
                                    toByte64(0xabcdefull));
        resumeAndExpectRead<Byte128>(*myProc, RegisterId::xmm0,
                                     toByte128(64.125));

        resumeAndExpectRead<long double>(*myProc, RegisterId::st0, 64.125L);
    }

} // namespace sdb::test
