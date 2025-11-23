#pragma once

#include <string>
#include <vector>

namespace sdb {

    std::vector<std::string> split(const std::string& str, char delimiter);
    bool isPrefix(const std::string& text, const std::string& prefix);

} // namespace sdb