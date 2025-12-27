#include "register_write.hpp"

#include <iostream>

namespace sdb {

    template <>
    std::optional<std::byte> toIntegral(std::string_view aValueToWrite) {

        auto myAsInt = toIntegral<std::uint8_t>(aValueToWrite);
        if (!myAsInt) {
            return std::nullopt;
        }

        return static_cast<std::byte>(*myAsInt);
    }

    RegisterValueT parseRegisterValue(const RegisterInfo& aRegisterInfo,
                                      const std::string& aValueToWrite) {

        switch (aRegisterInfo.theRegisterFormat) {
            case RegisterFormat::UInt:
                switch (aRegisterInfo.theSize) {
                    case 1:
                        return toIntegral<std::uint8_t>(aValueToWrite).value();
                    case 2:
                        return toIntegral<std::uint16_t>(aValueToWrite).value();
                    case 4:
                        return toIntegral<std::uint32_t>(aValueToWrite).value();
                    case 8:
                        return toIntegral<std::uint64_t>(aValueToWrite).value();
                }
                break;

            case RegisterFormat::DoubleFloat:
                return toFloat<double>(aValueToWrite).value();

            case RegisterFormat::LongDouble:
                return toFloat<long double>(aValueToWrite).value();

            case RegisterFormat::Vector:
                switch (aRegisterInfo.theSize) {
                    case 8:
                        return toVector<8>(aValueToWrite).value();
                    case 16:
                        return toVector<16>(aValueToWrite).value();
                    default:
                        Error::send("Invalid vector size");
                }
        }

        Error::send("Unreachable");
    }

    void handleRegisterWrite(Process& aProcess,
                             const std::string& aRegisterName,
                             const std::string& aValueToWrite) {

        auto& myRegisters = aProcess.getRegisters();

        const RegisterInfo& myRegisterInfo = findRegisterByName(aRegisterName);
        myRegisters.write(myRegisterInfo,
                          parseRegisterValue(myRegisterInfo, aValueToWrite));
    }

} // namespace sdb
