#pragma once

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

#include <process.hpp>

namespace sdb {

    template <typename T>
    std::optional<T> toIntegral(std::string_view aValueToWrite) {
        bool myIsHex = aValueToWrite.starts_with("0x");

        auto myBegin =
            myIsHex ? aValueToWrite.begin() + 2 : aValueToWrite.begin();

        T myResult{};
        auto myConverted = std::from_chars(myBegin, aValueToWrite.end(),
                                           myResult, myIsHex ? 16 : 10);

        if (myConverted.ptr != aValueToWrite.end()) {
            return std::nullopt;
        }

        return myResult;
    }

    template <>
    std::optional<std::byte> toIntegral(std::string_view aValueToWrite);

    template <std::floating_point T>
    std::optional<T> toFloat(std::string_view aValueToWrite) {

        T myResult{};
        auto myConverted = std::from_chars(aValueToWrite.begin(),
                                           aValueToWrite.end(), myResult);

        if (myConverted.ptr != aValueToWrite.end()) {
            return std::nullopt;
        }

        return myResult;
    }

    template <std::size_t VectorSizeT>
    std::optional<std::array<std::byte, VectorSizeT>>
    toVector(const std::string& aValueToWrite) {

        if (aValueToWrite.size() <= 2 || !aValueToWrite.starts_with('[') ||
            !aValueToWrite.ends_with(']')) {
            return std::nullopt;
        }

        std::array<std::byte, VectorSizeT> myResult{};

        auto myView = aValueToWrite | std::views::drop(1) |
                      std::views::take(aValueToWrite.size() - 2) |
                      std::views::split(',');

        auto to_sv = [](auto&& aRange) {
            return std::string_view{
                std::to_address(aRange.begin()),
                static_cast<size_t>(std::ranges::distance(aRange))};
        };

        size_t myIndex = 0;

        for (auto&& part : myView) {
            if (myIndex >= VectorSizeT) {
                return std::nullopt;
            }

            auto mySv = to_sv(part);
            if (mySv.size() != 4) {
                return std::nullopt;
            }

            auto myByte = toIntegral<std::byte>(mySv);
            if (!myByte) {
                return std::nullopt;
            }

            myResult[myIndex++] = *myByte;
        }

        if (myIndex != VectorSizeT) {
            return std::nullopt;
        }

        return myResult;
    }

    RegisterValueT parseRegisterValue(const RegisterInfo& aRegisterInfo,
                                      const std::string& aValueToWrite);

    void handleRegisterWrite(Process& aProcess,
                             const std::string& aRegisterName,
                             const std::string& aValueToWrite);

} // namespace sdb
