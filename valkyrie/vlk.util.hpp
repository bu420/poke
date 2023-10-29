#pragma once

#include <vector>
#include <string>
#include <functional>

#include "vlk.types.hpp"

namespace vlk {
    std::vector<std::string> string_split(const std::string& source, 
                                          const std::string& separator,
                                          bool allow_empty_substrings = false);
}
