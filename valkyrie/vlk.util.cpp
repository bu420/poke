#include "vlk.util.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>

using namespace vlk;

std::vector<std::string> vlk::string_split(std::string_view source, std::string_view separator,
                                           bool allow_empty_substrings) {
    assert(!source.empty());
    assert(!separator.empty());

    std::vector<std::string> tokens;

    auto pos       = source.begin();
    const auto end = source.end();

    while (true) {
        const auto next_separator_pos = std::find_first_of(pos, end, separator.begin(), separator.end());

        if (allow_empty_substrings || (!allow_empty_substrings && pos != next_separator_pos)) {
            tokens.push_back(std::string{pos, next_separator_pos});
        }

        if (next_separator_pos == end) {
            break;
        }

        if (allow_empty_substrings) {
            pos = next_separator_pos + separator.length();
        } else {
            pos = std::find_first_of(next_separator_pos, end, separator.begin(), separator.end(),
                                     [](auto &a, auto &b) { return a != b; });
        }
    }

    return tokens;
}

std::string vlk::uppercase(std::string_view str) {
    std::string new_str{str};

    for (auto &c : new_str) {
        c = std::toupper(c);
    }

    return new_str;
}

std::string vlk::lowercase(std::string_view str) {
    std::string new_str{str};

    for (auto &c : new_str) {
        c = std::tolower(c);
    }

    return new_str;
}

std::vector<u8> vlk::load_binary_file(std::filesystem::path path) {
    std::basic_ifstream<u8> file(path, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error(std::format("Valkyrie: file not found {}.", path.string()));
    }

    return std::vector<u8>((std::istreambuf_iterator<u8>(file)), std::istreambuf_iterator<u8>());
}