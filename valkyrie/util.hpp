#pragma once

#include <vector>
#include <string>
#include <functional>
#include <optional>

namespace poke {
    template<typename T> using optional_reference =
        std::optional<std::reference_wrapper<T>>;

    std::vector<std::string> string_split(const std::string& source, const std::string& separator);

    // Kind of like a string splitter but at each token you can decide to stop processing further.
    void string_get_next_token(
        const std::string& source, 
        const std::string& separator, 
        std::function<bool(const std::string& token)> callback);
}
