#include <bit.hpp>
#include <error.hpp>
#include <process.hpp>
#include <registers.hpp>

namespace sdb {
    RegisterValueT Registers::read(const RegisterInfo& aRegisterInfo) const {
        auto myRegisterInfoBytes = asBytes(theRegisterData);
        auto myDataAddr = myRegisterInfoBytes + aRegisterInfo.theOffset;

        switch (aRegisterInfo.theRegisterFormat) {
            case RegisterFormat::UInt: {
                switch (aRegisterInfo.theSize) {
                    case 1:
                        return fromBytes<std::uint8_t>(myDataAddr);
                    case 2:
                        return fromBytes<std::uint16_t>(myDataAddr);
                    case 4:
                        return fromBytes<std::uint32_t>(myDataAddr);
                    case 8:
                        return fromBytes<std::uint64_t>(myDataAddr);
                    default:
                        Error::send("Unexpected register size for UInt");
                }
            }
            case RegisterFormat::DoubleFloat:
                return fromBytes<double>(myDataAddr);
            case RegisterFormat::LongDouble:
                return fromBytes<long double>(myDataAddr);
            case RegisterFormat::Vector: {
                switch (aRegisterInfo.theSize) {
                    case 8:
                        return fromBytes<Byte64>(myDataAddr);
                    case 16:
                        return fromBytes<Byte128>(myDataAddr);
                    default:
                        Error::send("Unexpected regsiter size for Vector");
                }
            }
            default:
                Error::send("Unknown register format");
        }
    }

    void Registers::write(const RegisterInfo& aRegisterInfo,
                          RegisterValueT aValue) {
        auto myRegisterInfoAddr = asBytes(theRegisterData);
        auto myDataAddr = myRegisterInfoAddr + aRegisterInfo.theOffset;

        std::visit(
            [&](auto& aConcreteValue) {
                if (sizeof(aConcreteValue) <= aRegisterInfo.theSize) {
                    auto myWidenedValue = widen(aConcreteValue, aRegisterInfo);
                    auto myValueAsBytes = asBytes(myWidenedValue);

                    std::copy(myValueAsBytes,
                              myValueAsBytes + aRegisterInfo.theSize,
                              myDataAddr);
                } else {
                    Error::send("value too big for register size!");
                }
            },
            aValue);

        if (aRegisterInfo.theRegisterType == RegisterType::fpr) {
            theProcess.writeFloatingPointRegisters(theRegisterData.i387);
        }

        // rounds down to nearest multiple of 8 to avoid issues with reading
        // high 8-bit registers such as ah, bh, etc. Still valid because we are
        // reading the data 64 bits at a time anyway, so it should be starting
        // from the address of the 64 bits
        auto myAlignedOffset = aRegisterInfo.theOffset & ~0b111;

        theProcess.writeUserArea(
            myAlignedOffset,
            fromBytes<std::uint64_t>(myRegisterInfoAddr + myAlignedOffset));
    }

} // namespace sdb