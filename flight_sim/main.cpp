#include <valkyrie/vlk.math.hpp>
#include <valkyrie/vlk.gfx.hpp>
#include <valkyrie/vlk.system.hpp>
#include <valkyrie/vlk.ext.model.hpp>

#include <numbers>
#include <algorithm>
#include <iostream>

using namespace vlk;

int main() {
    const size_t width{600};
    const size_t height{600};

    initialize();

    auto model = load_model_obj("../assets/bullfrog.obj");

    if (!model.has_value()) {
        std::cout << "Failed to load assets." << std::endl;
        return -1;
    }

    window win{"flight sim", width, height};

    color_buffer color_buf{width, height};
    depth_buffer depth_buf{width, height};

    const mat4 projection_matrix = perspective(height / static_cast<float>(width),
                                               70 * (static_cast<float>(std::numbers::pi) / 180),
                                               0.001f,
                                               1000.f);

    const vec3f camera_pos{0.f, 10.f, -15.f};
    const mat4 view_matrix{look_at(camera_pos, {0.f, 10.f, 0.f}, {0.f, -1.f, 0.f})};

    while (!win.get_should_close()) {
        win.poll_events();

        color_buf.clear({std::byte{255}, std::byte{255}, std::byte{255}});
        depth_buf.clear(1.0f);

        mat4 model_matrix{1.0f};
        model_matrix = model_matrix.rotate_x(static_cast<float>(90.f * (std::numbers::pi / 180)));
        model_matrix = model_matrix.rotate_y(static_cast<float>(get_elapsed_time() * std::numbers::pi / 1500));
        //model_matrix = model_matrix.rotate_z(static_cast<float>(vlk::get_elapsed_time() * std::numbers::pi / 4000));
        model_matrix = model_matrix.translate({0.f, 0.f, static_cast<float>((std::sin(get_elapsed_time() / 750) + 1) * 17)});

        mat4 mvp_matrix{model_matrix * view_matrix * projection_matrix};
        mat3 normal_matrix{model_matrix.inverse().transpose()};

        //std::vector<bool> should_cull(model.value().meshes[0].faces.size());

        // Backface culling.
        /*for (int i = 0; i < model.value().meshes[0].faces.size(); i++) {
            // Pick any one of the triangle's points, put it in model space and check if the triangle should be culled.
            vlk::vec4f any_point_on_triangle = positions[model.value().meshes[0].faces[i].at(0)] * model;

            vlk::vec3f dir = (any_point_on_triangle.xyz() - camera_pos).normalize();
            vlk::vec3f normal = model.value().meshes[0].normals.value()[model.value().meshes[0].faces[i].at(0)] * normal_matrix;

            should_cull[i] = dir.dot(normal) >= 0;
        }*/

        render_model(*model, mvp_matrix, color_buf, depth_buf);

        // Render mesh.
        /*for (int i = 0; i < model3d.meshes[0].faces.size(); i++) {
            if (should_cull[i]) {
                continue;
            }

            const auto& face = model3d.meshes[0].faces[i];

            std::array<vlk::vec3f, 3> normals = {
                model3d.meshes[0].normals[face.indices[0].normal] * normal_matrix,
                model3d.meshes[0].normals[face.indices[1].normal] * normal_matrix,
                model3d.meshes[0].normals[face.indices[2].normal] * normal_matrix
            };

            std::array<vlk::vertex, 3> vertices = {
                vlk::vertex(positions[face.indices[0].pos], vlk::attribute(normals[0])),
                vlk::vertex(positions[face.indices[1].pos], vlk::attribute(normals[1])),
                vlk::vertex(positions[face.indices[2].pos], vlk::attribute(normals[2]))
            };

            vlk::render_triangle(color_buf, depth_buf, vertices,
                                 [](const vlk::vertex& vertex) -> vlk::byte3 {
                                     vlk::vec4f pos = vertex.position;

                                     vlk::vec3f normal(vertex.attributes[0].data.xyz());
                                     vlk::vec3f color((normal + 1.f) * 127.f);

                                     return {
                                         static_cast<vlk::byte>(color.x()),
                                         static_cast<vlk::byte>(color.y()),
                                         static_cast<vlk::byte>(color.z())
                                     };
                                 });
        }*/

        win.swap_buffers(color_buf);
    }
}