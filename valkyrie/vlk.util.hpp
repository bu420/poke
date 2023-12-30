#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <print>
#include <bitset>

#include "vlk.types.hpp"

namespace vlk {
    std::vector<std::string> string_split(std::string_view source, std::string_view separator,
                                          bool allow_empty_substrings = false);

    std::string uppercase(std::string_view str);
    std::string lowercase(std::string_view str);

    std::vector<u8> load_binary_file(std::filesystem::path path);

    template <typename T>
    class flag_set {
    public:
        bool is_set(T flag) const { return flags.test(static_cast<size_t>(flag)); }

        void set(T flag) { flags.set(static_cast<size_t>(flag)); }

        void unset(T flag) { flags.reset(static_cast<size_t>(flag)); }

        void clear() { flags.reset(); }

    private:
        std::bitset<sizeof(T) * 8> flags;
    };
}  // namespace vlk
