/* 
* This is an extension.
* It depends on "stb_image.h" to load material images.
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
            std::string material_name;

            bool has_tex_coords;
            bool has_normals;

            std::vector<vec3f> positions;
            std::vector<vec2f> tex_coords;
            std::vector<vec3f> normals;

            struct face {
                std::array<size_t, 3> position_indices;
                std::array<size_t, 3> tex_coord_indices;
                std::array<size_t, 3> normal_indices;
            };

            std::vector<face> faces;
        };

        std::vector<mesh> meshes;
        std::unordered_map<std::string, material> materials;
        std::unordered_map<std::string, image> images;
    };

    model parse_obj(const std::string& path);

    color_rgba default_model_pixel_shader(const vertex& vertex, 
                                          optional_ref<std::any> user_data);

    struct render_model_params {
        model model;
        optional_ref<std::any> user_data;
        mat4 mvp_matrix;
        mat3 normal_matrix;
        optional_ref<color_buffer> color_buf;
        optional_ref<depth_buffer> depth_buf;
        pixel_shader_callback pixel_shader = default_model_pixel_shader;
    };

    void render_model(const render_model_params& params);
}