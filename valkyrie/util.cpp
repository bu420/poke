#include "util.hpp"

#include <algorithm>

using namespace vlk;

std::vector<std::string> vlk::string_split(const std::string& source, 
                                           const std::string& separator) {
    std::vector<std::string> tokens;

    auto pos = source.begin();
    auto end = source.end();

    while (pos != end) {
        auto next_separator = std::find_first_of(pos, end, separator.begin(), separator.end());

        tokens.push_back(std::string(pos, next_separator));

        // Skip all consecutive occurences of the separator.
        pos = std::find_first_of(next_separator, end, separator.begin(), separator.end(),
            [](auto& a, auto& b) {
                return a != b;
            });
    }

    return tokens;
}

void vlk::string_split(const std::string& source,
                       const std::string& separator,
                       std::function<bool(const std::string& token)> callback) {
    auto pos = source.begin();
    auto end = source.end();

    while (pos != end) {
        auto next_separator = std::find_first_of(pos, end, separator.begin(), separator.end());

        if (!callback(std::string(pos, next_separator))) {
            return;
        }

        // Skip all consecutive occurences of the separator.
        pos = std::find_first_of(next_separator, end, separator.begin(), separator.end(),
            [](auto& a, auto& b) {
                return a != b;
            });
    }
}