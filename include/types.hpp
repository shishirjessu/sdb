#pragma once

#include <array>
#include <cstdint>
#include <variant>

namespace sdb {
    using Byte64 = std::array<std::byte, 8>;
    using Byte128 = std::array<std::byte, 16>;

    using RegisterValueT =
        std::variant<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
                     std::int8_t, std::int16_t, std::int32_t, std::int64_t,
                     float, double, long double, Byte64, Byte128>;

    template <typename EnumT>
    auto toUnderlying(EnumT anEnum) {
        return static_cast<std::underlying_type_t<EnumT>>(anEnum);
    }

}; // namespace sdb