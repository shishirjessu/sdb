#pragma once

#include <string_view>
#include <vector>

namespace sdb {
    inline std::string_view toStringView(const std::byte* aData,
                                         std::size_t aSize) {
        return {reinterpret_cast<const char*>(aData), aSize};
    }

    inline std::string_view toStringView(const std::vector<std::byte>& aBytes) {
        return toStringView(aBytes.data(), aBytes.size());
    }
} // namespace sdb