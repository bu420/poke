#include "vlk.ext.model.hpp"

#pragma warning(disable : 5219)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(default : 5219)
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <format>

#include "vlk.util.hpp"

using namespace vlk;

std::string read_text_file(const std::string& path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error(
            std::format("Valkyrie: file not found error: {}", path));
    }

    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

std::vector<std::vector<std::string>> tokenize_obj_or_mtl(const std::string& file_content) {
    std::vector<std::vector<std::string>> tokens;

    for (const auto& line : string_split(file_content, "\n")) {
        const auto& line_tokens = string_split(line, " ");

        if (line_tokens.empty()) {
            continue;
        }

        if (line_tokens.at(0) == "#") {
            continue;
        }

        tokens.emplace_back(line_tokens);
    }

    return tokens;
}

image load_image(const std::string& path) {
    i32 width;
    i32 height;
    i32 channels;
    std::byte* image_data =
        reinterpret_cast<std::byte*>(stbi_load(path.c_str(),
                                               &width,
                                               &height,
                                               &channels, 0));

    if (!image_data) {
        throw std::runtime_error(
            std::format("Valkyrie: file not found error: {}", path));
    }

    return image{
        .width{static_cast<size_t>(width)},
        .height{static_cast<size_t>(height)},
        .channels{static_cast<u8>(channels)},
        .data{std::vector<std::byte>(image_data, image_data + width * height * channels)}
    };
}

std::vector<std::pair<std::string, model::material>> parse_mtl(const std::string& path) {
    std::vector<std::pair<std::string, model::material>> materials;

    const auto tokens = tokenize_obj_or_mtl(read_text_file(path));

    /*
    * An MTL can contain multiple materials.
    * Each new material starts with: newmtl <material name>
    */

    auto current_newmtl_pos = std::find_if(tokens.begin(),
                                           tokens.end(),
                                           [](const auto& line_tokens) {
                                               return line_tokens.at(0) == "newmtl";
                                           });

    while (current_newmtl_pos != tokens.end()) {
        auto next_newmtl_pos = std::find_if(current_newmtl_pos,
                                            tokens.end(),
                                            [](const auto& line_tokens) {
                                                return line_tokens.at(0) == "newmtl";
                                            });

        std::string name{current_newmtl_pos->at(1)};
        model::material material;

        std::for_each(current_newmtl_pos,
                      next_newmtl_pos,
                      [&](const auto& line_tokens) {
                          if (line_tokens.at(0) == "map_Ka") {
                              material.albedo_map_image_path = line_tokens.at(1);
                          }

                          else if (line_tokens.at(0) == "norm") {
                              material.normal_map_image_path = line_tokens.at(1);
                          }

                          else if (line_tokens.at(0) == "Pr" or line_tokens.at(0) == "map_Pr") {
                              material.roughness_map_image_path = line_tokens.at(1);
                          }

                          else if (line_tokens.at(0) == "Pm" or line_tokens.at(0) == "map_Pm") {
                              material.metallic_map_image_path = line_tokens.at(1);
                          }

                          else if (line_tokens.at(0) == "map_d") {
                              material.opacity_map_image_path = line_tokens.at(1);
                          }
                      });

        materials.emplace_back(std::make_pair(name, material));

        current_newmtl_pos = next_newmtl_pos;
    }

    return materials;
}

model vlk::parse_obj(const std::string& path) {
    model model;

    const auto tokens = tokenize_obj_or_mtl(read_text_file(path));

    // Parse MTL files.

    auto mtllib_pos = std::find_if(tokens.begin(),
                                   tokens.end(),
                                   [](const auto& line_tokens) {
                                       return line_tokens.at(0) == "mtllib";
                                   });

    while (mtllib_pos != tokens.end()) {
        const std::string& mtl_path = mtllib_pos->at(1);

        std::vector<std::pair<std::string, model::material>> materials = parse_mtl(mtl_path);

        for (const auto& [material_name, material] : materials) {
            model.materials.insert(std::make_pair(material_name, material));

            auto load_image_if_not_already_loaded = [&](const std::string& path) {
                if (!model.images.contains(path)) {
                    model.images.insert(std::make_pair(path, load_image(path)));
                }
            };

            if (material.albedo_map_image_path) {
                load_image_if_not_already_loaded(*material.albedo_map_image_path);
            }
            if (material.normal_map_image_path) {
                load_image_if_not_already_loaded(*material.normal_map_image_path);
            }
            if (material.roughness_map_image_path) {
                load_image_if_not_already_loaded(*material.roughness_map_image_path);
            }
            if (material.metallic_map_image_path) {
                load_image_if_not_already_loaded(*material.metallic_map_image_path);
            }
            if (material.opacity_map_image_path) {
                load_image_if_not_already_loaded(*material.opacity_map_image_path);
            }
        }

        mtllib_pos = std::find_if(mtllib_pos,
                                  tokens.end(),
                                  [](const auto& line_tokens) {
                                      return line_tokens.at(0) == "mtllib";
                                  });
    }

    // Parse meshes.

    auto current_usemtl_pos = std::find_if(tokens.begin(),
                                           tokens.end(),
                                           [](const auto& line_tokens) {
                                               return line_tokens.at(0) == "usemtl";
                                           });

    while (current_usemtl_pos != tokens.end()) {
        auto next_usemtl_pos = std::find_if(current_usemtl_pos,
                                            tokens.end(),
                                            [](const auto& line_tokens) {
                                                return line_tokens.at(0) == "usemtl";
                                            });

        model::mesh mesh{
            .material_name{current_usemtl_pos->at(1)},
            .has_tex_coords{false},
            .has_normals{false}
        };

        std::for_each(current_usemtl_pos,
                      next_usemtl_pos,
                      [&](const auto& line_tokens) {
                          if (line_tokens.at(0) == "vp") {
                              mesh.positions.emplace_back(vec3f(std::stof(line_tokens.at(1)),
                                                                std::stof(line_tokens.at(2)),
                                                                std::stof(line_tokens.at(3))));
                          }

                          else if (line_tokens.at(0) == "vt") {
                              mesh.has_tex_coords = true;
                              mesh.tex_coords.emplace_back(vec2f(std::stof(line_tokens.at(1)),
                                                                 std::stof(line_tokens.at(2))));
                          }

                          else if (line_tokens.at(0) == "vn") {
                              mesh.has_normals = true;
                              mesh.normals.emplace_back(vec3f(std::stof(line_tokens.at(1)),
                                                              std::stof(line_tokens.at(2)),
                                                              std::stof(line_tokens.at(3))));
                          }

                          else if (line_tokens.at(0) == "f") {
                              size_t triangle_count{line_tokens.size() - 3};

                              for (size_t i{0}; i < triangle_count; ++i) {
                                  model::mesh::face face{};

                                  for (size_t j{0}; j < 3; ++j) {
                                      size_t token_index{(i * 2 + j) % (line_tokens.size() - 1)};

                                      const auto indices{string_split(line_tokens.at(token_index), "/", false)};

                                      face.position_indices.at(j) = std::stoi(indices.at(0));

                                      if (indices.size() == 2) {
                                          face.tex_coord_indices.at(j) = std::stoi(indices.at(1));
                                      }
                                      else if (indices.size() == 3) {
                                          if (!indices.at(1).empty()) {
                                              face.tex_coord_indices.at(j) = std::stoi(indices.at(1));
                                          }

                                          face.normal_indices.at(j) = std::stoi(indices.at(2));
                                      }
                                  }
                              }

                              mesh.faces.emplace_back();
                          }
                      });

        model.meshes.emplace_back(mesh);

        current_usemtl_pos = next_usemtl_pos;
    }

    return model;
}

color_rgba vlk::default_model_pixel_shader(const vertex& vertex,
                                           optional_ref<std::any> user_data) {
    return color_rgba{std::byte{255},
                      std::byte{0},
                      std::byte{255},
                      std::byte{255}};
}

void vlk::render_model(const render_model_params& params) {
    for (auto& mesh : params.model.meshes) {
        for (auto& face : mesh.faces) {
            const std::array<vertex, 3> vertices{
                vertex{
                    vec4f{mesh.positions[face.position_indices[0]], 1.0f} * params.mvp_matrix,
                    attribute{mesh.tex_coords[face.tex_coord_indices[0]]},
                    attribute{mesh.normals[face.normal_indices[0]] * params.normal_matrix}
                },

                vertex{
                    vec4f{mesh.positions[face.position_indices[1]], 1.0f} * params.mvp_matrix,
                    attribute{mesh.tex_coords[face.tex_coord_indices[1]]},
                    attribute{mesh.normals[face.normal_indices[1]] * params.normal_matrix}
                },

                vertex{
                    vec4f{mesh.positions[face.position_indices[2]], 1.0f} * params.mvp_matrix,
                    attribute{mesh.tex_coords[face.tex_coord_indices[2]]},
                    attribute{mesh.normals[face.normal_indices[2]] * params.normal_matrix}
                }
            };

            render_triangle(
                {
                    .vertices{vertices},
                    .user_data{params.user_data},
                    .color_buf{params.color_buf},
                    .depth_buf{params.depth_buf},
                    .pixel_shader{params.pixel_shader}
                });
        }
    }
}