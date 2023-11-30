#pragma once

#include <vector>
#include <string>
#include <functional>
#include <print>

#include "vlk.types.hpp"

namespace vlk {
    std::vector<std::string> string_split(std::string_view source, 
                                          std::string_view separator,
                                          bool allow_empty_substrings = false);

    std::string uppercase(std::string_view str);
    std::string lowercase(std::string_view str);
}
