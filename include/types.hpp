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

    enum class VirtualAddress : std::uint64_t {};

    inline auto operator<=>(VirtualAddress a, VirtualAddress b) noexcept {
        using U = std::underlying_type_t<VirtualAddress>;
        return static_cast<U>(a) <=> static_cast<U>(b);
    }

    inline constexpr bool operator==(VirtualAddress a,
                                     VirtualAddress b) noexcept {
        return static_cast<int>(a) == static_cast<int>(b);
    }

    inline constexpr VirtualAddress operator+(VirtualAddress a,
                                              std::uint64_t b) noexcept {
        return static_cast<VirtualAddress>(static_cast<std::uint64_t>(a) + b);
    }

    inline constexpr VirtualAddress operator-(VirtualAddress a,
                                              std::uint64_t b) noexcept {
        return static_cast<VirtualAddress>(static_cast<std::uint64_t>(a) - b);
    }

    inline constexpr VirtualAddress& operator+=(VirtualAddress& a,
                                                std::uint64_t b) noexcept {
        a = a + b;
        return a;
    }

    inline constexpr VirtualAddress& operator-=(VirtualAddress& a,
                                                std::uint64_t b) noexcept {
        a = a - b;
        return a;
    }

    template <typename EnumT>
    auto toUnderlying(EnumT anEnum) {
        return static_cast<std::underlying_type_t<EnumT>>(anEnum);
    }

}; // namespace sdb