#include <ranges>
#include <string>
#include <vector>

namespace sdb {

    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> result{};

        for (auto&& part : str | std::views::split(delimiter)) {
            result.emplace_back(part.begin(), part.end());
        }

        return result;
    }

    bool isPrefix(const std::string& text, const std::string& prefix) {
        return text.rfind(prefix, 0) == 0;
    }

} // namespace sdb