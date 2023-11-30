#pragma once

#include <filesystem>

#include "vlk.gfx.hpp"

namespace vlk {
    model parse_obj(std::filesystem::path path);
}