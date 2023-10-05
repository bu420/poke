#include "vlk.ext.model.hpp"

#pragma warning(disable : 5219)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(default : 5219)

using namespace vlk;

std::optional<model> vlk::load_model_obj(const std::string& path) {
    /*i32 width;
    i32 height;
    i32 channels;
    std::byte* image_data =
        reinterpret_cast<std::byte*>(stbi_load(path.c_str(),
                                               &width,
                                               &height,
                                               &channels, 0));
    assert(image_data);

    image image;
    image.width = width;
    image.height = height;
    image.channels = channels;
    image.data = std::vector<std::byte>(image_data,
                                        image_data + width * height * channels);*/
}

std::optional<byte3> vlk::default_model_pixel_shader(const vertex& vertex) {
    return byte3{std::byte{255}, std::byte{0}, std::byte{255}};
}

void vlk::render_model(const model& model,
                       const mat4& transformation,
                       optional_ref<color_buffer> color_buf,
                       optional_ref<depth_buffer> depth_buf,
                       pixel_shader_callback pixel_shader) {
    for (auto& mesh : model.meshes) {
        for (auto& face : mesh.faces) {
            auto to_vec4f = [](const vec3f& v) {
                return vec4f{v.x(), v.y(), v.z(), 1.0f};
            };

            std::array<vertex, 3> vertices {
                vertex{to_vec4f(mesh.positions.at(face.at(0))) * transformation, 
                       attribute{mesh.tex_coords->at(face.at(0))}, 
                       attribute{mesh.normals->at(face.at(0))}},

                vertex{to_vec4f(mesh.positions.at(face.at(1))) * transformation,
                       attribute{mesh.tex_coords->at(face.at(1))},
                       attribute{mesh.normals->at(face.at(2))}},

                vertex{to_vec4f(mesh.positions.at(face.at(2))) * transformation,
                       attribute{mesh.tex_coords->at(face.at(2))},
                       attribute{mesh.normals->at(face.at(2))}}
            };

            render_triangle(vertices, color_buf, depth_buf, pixel_shader);
        }
    }
}