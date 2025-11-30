#pragma once

#include <cstddef>
#include <cstring>
#include <types.hpp>

namespace sdb {

    template <typename DestTypeT>
    DestTypeT fromBytes(const std::byte* aBytes) {
        DestTypeT myDest;
        std::memcpy(std::addressof(myDest), aBytes, sizeof(myDest));
        return myDest;
    }

    template <typename SrcTypeT>
    std::byte* asBytes(SrcTypeT& aSrc) {
        return reinterpret_cast<std::byte*>(std::addressof(aSrc));
    }

    template <typename SrcTypeT>
    const std::byte* asBytes(const SrcTypeT& aSrc) {
        return reinterpret_cast<const std::byte*>(std::addressof(aSrc));
    }

    template <typename SrcTypeT>
    Byte64 toByte64(SrcTypeT aSrcType) {
        Byte64 myRes{};
        std::memcpy(std::addressof(myRes), std::addressof(aSrcType),
                    sizeof(SrcTypeT));
        return myRes;
    }

    template <typename SrcTypeT>
    Byte128 toByte128(SrcTypeT aSrcType) {
        Byte128 myRes{};
        std::memcpy(std::addressof(myRes), std::addressof(aSrcType),
                    sizeof(SrcTypeT));
        return myRes;
    }

} // namespace sdb