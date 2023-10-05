#pragma once

#include <vector>
#include <string>
#include <functional>

#include "vlk.types.hpp"

namespace vlk {
    std::vector<std::string> string_split(const std::string& source, 
                                          const std::string& separator);

    // For each token you can decide to stop processing further by returning false.
    void string_split(const std::string& source, 
                      const std::string& separator,
                      std::function<bool(const std::string& token)> callback);
}
