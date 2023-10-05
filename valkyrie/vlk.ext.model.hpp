/* 
* This is an extension.
* This depends on "stb_image.h" to load material images.
*/

#pragma once

#include <optional>
#include <string>
#include <vector>
#include <unordered_map>

#include "vlk.vec.hpp"
#include "vlk.gfx.hpp"

namespace vlk {
    struct model {
        struct material {
            std::optional<std::string> albedo_map_image_path;
            std::optional<std::string> normal_map_image_path;
            std::optional<std::string> roughness_map_image_path;
            std::optional<std::string> metallic_map_image_path;
            std::optional<std::string> opacity_map_image_path;
        };

        struct mesh {
            std::vector<vec3f> positions;
            std::optional<std::vector<vec2f>> tex_coords;
            std::optional<std::vector<vec3f>> normals;

            std::vector<std::array<size_t, 3>> faces;

            std::optional<size_t> material_index;
        };

        std::vector<mesh> meshes;
        std::vector<material> materials;
        std::unordered_map<std::string, image> images;
    };

    std::optional<model> load_model_obj(const std::string& path);

    std::optional<byte3> default_model_pixel_shader(const vertex&);

    void render_model(const model& model,
                      const mat4& transformation,
                      optional_ref<color_buffer> color_buf,
                      optional_ref<depth_buffer> depth_buf,
                      pixel_shader_callback pixel_shader = default_model_pixel_shader);
}