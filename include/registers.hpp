#pragma once

#include <bit.hpp>
#include <libsdb/register_info.hpp>
#include <memory>
#include <optional>
#include <types.hpp>
#include <variant>

#include <sys/user.h>

namespace sdb {
    class Process;

    class Registers {

      public:
        Registers(Process& aProcess) : theProcess{aProcess} {
        }

        Registers(const Registers& other) = delete;
        Registers(Registers&& other) = delete;

        Registers& operator=(const Registers& other) = delete;
        Registers& operator=(Registers&& other) = delete;

        RegisterValueT read(const RegisterInfo& aRegisterInfo) const;
        void write(const RegisterInfo& aRegisterInfo, RegisterValueT aValue);

        user& getRegisterData() {
            return theRegisterData;
        }

      private:
        user theRegisterData{};
        Process& theProcess;

        // If the size of the value does not fit into the register
        // cleanly (for example, a 32 bit value going into a 64 bit register),
        // then it must be widened. We can't just store it directly into the
        // register, because signed integers need to be sign extended (converted
        // into two's complement) and floating point values need to be
        // represented in IEEE754. Therefore, we need to apply these static
        // casts.
        //
        // We convert to Byte128 because some of the FP registers are 80 bytes.
        template <typename T>
        Byte128 widen(T aValue, const RegisterInfo& aRegisterInfo) {
            RegisterFormat myRegisterFormat = aRegisterInfo.theRegisterFormat;

            if constexpr (std::is_floating_point_v<T>) {
                if (myRegisterFormat == RegisterFormat::DoubleFloat) {
                    return toByte128(static_cast<double>(aValue));
                } else if (myRegisterFormat == RegisterFormat::LongDouble) {
                    return toByte128(static_cast<long double>(aValue));
                } else if (myRegisterFormat == RegisterFormat::Vector) {
                    return toByte128(aValue);
                }
            } else if constexpr (std::is_signed_v<T>) {
                if (aRegisterInfo.theSize == 8) {
                    return toByte128(static_cast<std::int64_t>(aValue));
                } else if (aRegisterInfo.theSize == 4) {
                    return toByte128(static_cast<std::int32_t>(aValue));
                } else if (aRegisterInfo.theSize == 2) {
                    return toByte128(static_cast<std::int16_t>(aValue));
                } else if (aRegisterInfo.theSize == 1) {
                    return toByte128(static_cast<std::int8_t>(aValue));
                }
            } else {
                return toByte128(aValue);
            }
        }
    };
} // namespace sdb
