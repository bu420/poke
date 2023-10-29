#include "vlk.util.hpp"

#include <algorithm>
#include <cassert>

using namespace vlk;

std::vector<std::string> vlk::string_split(const std::string& source,
                                           const std::string& separator,
                                           bool allow_empty_substrings) {
    assert(not source.empty());
    assert(not separator.empty());

    std::vector<std::string> tokens;

    auto pos{source.begin()};
    const auto end{source.end()};

    while (true) {
        const auto next_separator_pos = std::find_first_of(pos, end, separator.begin(), separator.end());

        if (allow_empty_substrings or (not allow_empty_substrings and pos != next_separator_pos)) {
            tokens.push_back(std::string{pos, next_separator_pos});
        }

        if (next_separator_pos == end) {
            break;
        }

        if (allow_empty_substrings) {
            pos = next_separator_pos + separator.length();
        }
        else {
            pos = std::find_first_of(next_separator_pos,
                                     end,
                                     separator.begin(),
                                     separator.end(),
                                     [](auto& a, auto& b) {
                                         return a != b;
                                     });
        }
    }

    return tokens;
}
