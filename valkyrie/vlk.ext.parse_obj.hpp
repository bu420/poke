/*
* This is an extension.
* It depends on "stb_image.h" to load material images.
*/

#pragma once

#include <string>
#include <stdexcept>

#include "vlk.gfx.hpp"

namespace vlk {
    model parse_obj(const std::string& path);
}