#include "vlk.types.hpp"

std::byte vlk::operator ""_byte (size_t value) {
    return static_cast<std::byte>(value);
}