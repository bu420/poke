#include "vlk.parse_obj.hpp"

#include <string_view>
#include <fstream>
#include <sstream>
#include <format>
#include <stdexcept>
#include <print>

#include "vlk.util.hpp"
#include "vlk.system.hpp"

using namespace vlk;

std::string read_text_file(std::filesystem::path path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error(
            std::format("Valkyrie: file not found error: {}", path.string()));
    }

    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

std::vector<std::vector<std::string>> tokenize_obj_or_mtl(std::string_view file_content) {
    std::vector<std::vector<std::string>> tokens;

    for (auto& line : string_split(file_content, "\n")) {
        // Remove all tabs.
        line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());

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

void parse_mtl(std::filesystem::path path,
               std::vector<std::pair<std::string, model::material>>& materials,
               std::vector<std::pair<std::filesystem::path, image>>& images) {
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
        auto next_newmtl_pos = std::find_if(current_newmtl_pos + 1,
                                            tokens.end(),
                                            [](const auto& line_tokens) {
                                                return line_tokens.at(0) == "newmtl";
                                            });

        const std::string& material_name = current_newmtl_pos->at(1);

        const auto material_pos = std::find_if(materials.begin(),
                                               materials.end(),
                                               [&](const auto& elem) {
                                                   return elem.first == material_name;
                                               });

        if (material_pos != materials.end()) {
            throw std::runtime_error(
                std::format("Valkyrie: error when parsing mtl {}: material {} is already defined.", path.string(), material_name));
        }

        model::material material{.name{material_name}};

        const std::unordered_map<std::string, size_t&> indices{
            {"map_Ka", material.albedo_map_index},
            {"map_Kd", material.albedo_map_index},

            {"norm", material.normal_map_index},
            {"bump", material.normal_map_index},
            {"map_bump", material.normal_map_index},

            {"Pr", material.roughness_map_index},
            {"map_Pr", material.roughness_map_index},

            {"Pm", material.metallic_map_index},
            {"map_Pm", material.metallic_map_index},

            {"map_d", material.opacity_map_index}
        };

        std::for_each(current_newmtl_pos,
                      next_newmtl_pos,
                      [&](const auto& line_tokens) {
                          const std::string token = line_tokens.at(0);

                          auto index_pos = indices.find(token);

                          if (index_pos != indices.end()) {
                              const auto image_path = path.remove_filename() / line_tokens.at(1);

                              const auto image_pos = std::find_if(images.begin(),
                                                                  images.end(),
                                                                  [&](const auto& elem) {
                                                                      return elem.first == image_path;
                                                                  });

                              const size_t index = [&]() -> size_t {
                                  // Check if an image with this path has already been loaded.
                                  if (image_pos != images.end()) {
                                      return image_pos - images.begin();

                                  }
                                  else {
                                      images.push_back(std::make_pair(image_path, load_image(image_path)));
                                      return images.size() - 1;
                                  }
                              }();

                              size_t& image_map_index = index_pos->second;
                              image_map_index = index;
                          }
                      });

        materials.push_back(std::make_pair(material.name, material));

        current_newmtl_pos = next_newmtl_pos;
    }
}

model vlk::parse_obj(std::filesystem::path path) {
    model model;

    const auto tokens = tokenize_obj_or_mtl(read_text_file(path));

    // Parse MTL files.

    std::vector<std::pair<std::string, model::material>> materials;
    std::vector<std::pair<std::filesystem::path, image>> images;

    auto mtllib_pos = std::find_if(tokens.begin(),
                                   tokens.end(),
                                   [](const auto& line_tokens) {
                                       return line_tokens.at(0) == "mtllib";
                                   });

    while (mtllib_pos != tokens.end()) {
        const auto mtl_path = path.remove_filename() / mtllib_pos->at(1);

        parse_mtl(mtl_path, materials, images);

        mtllib_pos = std::find_if(mtllib_pos + 1,
                                  tokens.end(),
                                  [](const auto& line_tokens) {
                                      return line_tokens.at(0) == "mtllib";
                                  });
    }

    for (const auto& [_, material] : materials) {
        model.materials.push_back(material);
    }
    for (const auto& [_, image] : images) {
        model.images.push_back(image);
    }

    // Parse meshes.

    for (const auto& line_tokens : tokens) {
        if (line_tokens.at(0) == "v") {
            model.positions.emplace_back(vec3f(std::stof(line_tokens.at(1)),
                                               std::stof(line_tokens.at(2)),
                                               std::stof(line_tokens.at(3))));
        }

        else if (line_tokens.at(0) == "vt") {
            model.tex_coords.emplace_back(vec2f(std::stof(line_tokens.at(1)),
                                                std::stof(line_tokens.at(2))));
        }

        else if (line_tokens.at(0) == "vn") {
            model.normals.emplace_back(vec3f(std::stof(line_tokens.at(1)),
                                             std::stof(line_tokens.at(2)),
                                             std::stof(line_tokens.at(3))));
        }
    }

    auto current_usemtl_pos = std::find_if(tokens.begin(),
                                           tokens.end(),
                                           [](const auto& line_tokens) {
                                               return line_tokens.at(0) == "usemtl";
                                           });

    // Edge case: if there's zero "usemtl"'s found, parse everything as a single mesh without a material.
    if (current_usemtl_pos == tokens.end()) {
        model::mesh mesh{
            .material_index = model::no_index,
            .has_tex_coords = false,
            .has_normals = false
        };

        std::for_each(tokens.begin(),
                      tokens.end(),
                      [&](const auto &line_tokens) {
            if (line_tokens.at(0) == "f") {
                size_t triangle_count{line_tokens.size() - 3};

                for (size_t i{0}; i < triangle_count; ++i) {
                    model::mesh::face face{};

                    for (size_t j{0}; j < 3; ++j) {
                        size_t token_index{1 + ((i * 2 + j) % (line_tokens.size() - 1))};

                        const auto indices{string_split(line_tokens.at(token_index), "/", true)};

                        face.position_indices.at(j) = std::stoi(indices.at(0)) - 1;

                        if (indices.size() == 2) {
                            face.tex_coord_indices.at(j) = std::stoi(indices.at(1)) - 1;
                            mesh.has_tex_coords = true;
                        }
                        else if (indices.size() == 3) {
                            if (!indices.at(1).empty()) {
                                face.tex_coord_indices.at(j) = std::stoi(indices.at(1)) - 1;
                            }

                            face.normal_indices.at(j) = std::stoi(indices.at(2)) - 1;

                            mesh.has_tex_coords = true;
                            mesh.has_normals = true;
                        }
                    }

                    mesh.faces.emplace_back(face);
                }
            }
        });

        model.meshes.emplace_back(mesh);
        return model;
    }

    while (current_usemtl_pos != tokens.end()) {
        auto next_usemtl_pos = std::find_if(current_usemtl_pos + 1,
                                            tokens.end(),
                                            [](const auto& line_tokens) {
                                                return line_tokens.at(0) == "usemtl";
                                            });

        const std::string& material_name = current_usemtl_pos->at(1);

        auto material_pos = std::find_if(materials.begin(),
                                         materials.end(),
                                         [&](const auto& elem) {
                                             return elem.first == material_name;
                                         });

        if (material_pos == materials.end()) {
            throw std::runtime_error(
                std::format("Valkyrie: error when parsing mesh {}: material {} does not exist.", path.string(), material_name));
        }

        const size_t material_index = material_pos - materials.begin();

        model::mesh mesh{
            .material_index{material_index},
            .has_tex_coords{false},
            .has_normals{false}
        };

        std::for_each(current_usemtl_pos,
                      next_usemtl_pos,
                      [&](const auto& line_tokens) {
                          if (line_tokens.at(0) == "f") {
                              size_t triangle_count{line_tokens.size() - 3};

                              for (size_t i{0}; i < triangle_count; ++i) {
                                  model::mesh::face face{};

                                  for (size_t j{0}; j < 3; ++j) {
                                      size_t token_index{1 + ((i * 2 + j) % (line_tokens.size() - 1))};

                                      const auto indices{string_split(line_tokens.at(token_index), "/", true)};

                                      face.position_indices.at(j) = std::stoi(indices.at(0)) - 1;

                                      if (indices.size() == 2) {
                                          face.tex_coord_indices.at(j) = std::stoi(indices.at(1)) - 1;
                                          mesh.has_tex_coords = true;
                                      }
                                      else if (indices.size() == 3) {
                                          if (!indices.at(1).empty()) {
                                              face.tex_coord_indices.at(j) = std::stoi(indices.at(1)) - 1;
                                          }

                                          face.normal_indices.at(j) = std::stoi(indices.at(2)) - 1;

                                          mesh.has_tex_coords = true;
                                          mesh.has_normals = true;
                                      }
                                  }

                                  mesh.faces.emplace_back(face);
                              }
                          }
                      });

        model.meshes.emplace_back(mesh);

        current_usemtl_pos = next_usemtl_pos;
    }

    return model;
}