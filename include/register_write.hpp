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

    template <typename Container, typename T = std::byte>
    std::optional<Container> toVectorImpl(const std::string& aValueToWrite,
                                          size_t expectedSize = 0) {
        if (aValueToWrite.size() <= 2 || !aValueToWrite.starts_with('[') ||
            !aValueToWrite.ends_with(']')) {
            return std::nullopt;
        }

        Container result;

        auto myView = aValueToWrite | std::views::drop(1) |
                      std::views::take(aValueToWrite.size() - 2) |
                      std::views::split(',');

        auto to_sv = [](auto&& aRange) {
            return std::string_view{
                std::to_address(aRange.begin()),
                static_cast<size_t>(std::ranges::distance(aRange))};
        };

        size_t count = 0;
        for (auto&& part : myView) {
            if (expectedSize && count >= expectedSize) {
                return std::nullopt; // too many elements for fixed-size array
            }

            auto mySv = to_sv(part);
            if (mySv.size() != 4) {
                return std::nullopt; // enforce "0xNN" format
            }

            auto myByte = toIntegral<T>(mySv);
            if (!myByte) {
                return std::nullopt;
            }

            if constexpr (std::is_same_v<Container, std::vector<T>>) {
                result.push_back(*myByte);
            } else {
                result[count] = *myByte;
            }
            ++count;
        }

        if (expectedSize && count != expectedSize) {
            return std::nullopt;
        }

        return result;
    }

    template <std::size_t VectorSizeT>
    std::optional<std::array<std::byte, VectorSizeT>>
    toVector(const std::string& aValueToWrite) {
        return toVectorImpl<std::array<std::byte, VectorSizeT>>(aValueToWrite,
                                                                VectorSizeT);
    }

    std::optional<std::vector<std::byte>> inline toVectorDynamic(
        const std::string& aValueToWrite) {
        return toVectorImpl<std::vector<std::byte>>(aValueToWrite);
    }

    RegisterValueT parseRegisterValue(const RegisterInfo& aRegisterInfo,
                                      const std::string& aValueToWrite);

    void handleRegisterWrite(Process& aProcess,
                             const std::string& aRegisterName,
                             const std::string& aValueToWrite);

} // namespace sdb
