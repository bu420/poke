#include <valkyrie/math.hpp>
#include <valkyrie/gfx.hpp>
#include <platform/platform.hpp>

#include <numbers>
#include <algorithm>
#include <iostream>

#define W 600
#define H 600

int main() {
    vlk::initialize();

    vlk::model model3d;
    std::cout << "Loading assets..." << std::endl;
    if (!model3d.load("../assets/earth/uv_planet.vmod")) {
        std::cout << "Failed to load assets." << std::endl;
        return -1;
    }
    std::cout << "Done." << std::endl;

    vlk::window win("valkyrie", W, H);

    vlk::color_buffer color_buf(W, H);
    vlk::depth_buffer depth_buf(W, H);

    const vlk::mat4 projection = vlk::perspective(
        H / static_cast<float>(W),
        70 * (static_cast<float>(std::numbers::pi) / 180),
        20.f,
        1000.f);

    const vlk::vec3f camera_pos(0.f, 10.f, -15.f);
    const vlk::mat4 view = vlk::look_at(camera_pos, { 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f });

    while (!win.get_should_close()) {
        win.poll_events();

        color_buf.clear({ 255, 255, 255 });
        depth_buf.clear(1.0f);

        vlk::mat4 model(1);
        model = model.rotate_x(static_cast<float>(90.f * (std::numbers::pi / 180)));
        model = model.rotate_y(static_cast<float>(vlk::get_elapsed_time() * std::numbers::pi / 1500));
        //model = model.rotate_z(static_cast<float>(vlk::get_elapsed_time() * std::numbers::pi / 4000));
        model = model.translate({ 0.f, 0.f, static_cast<float>((std::sin(vlk::get_elapsed_time() / 750) + 1) * 17) });

        vlk::mat4 mvp = model * view * projection;
        vlk::mat3 normal_matrix = model.inverse().transpose();

        // Make a copy that we can mutate.
        std::vector<vlk::vec4f> positions(model3d.meshes[0].positions.size());
        for (int i = 0; i < model3d.meshes[0].positions.size(); i++) {
            positions[i].x() = model3d.meshes[0].positions[i].x();
            positions[i].y() = model3d.meshes[0].positions[i].y();
            positions[i].z() = model3d.meshes[0].positions[i].z();
            positions[i].w() = 1.f;
        }

        std::vector<bool> should_cull(model3d.meshes[0].faces.size());

        // Backface culling.
        for (int i = 0; i < model3d.meshes[0].faces.size(); i++) {
            // Pick any one of the triangle's points, put it in model space and check if the triangle should be culled.
            vlk::vec4f any_point_on_triangle = positions[model3d.meshes[0].faces[i].indices[0].pos] * model;

            vlk::vec3f dir = (any_point_on_triangle.xyz() - camera_pos).normalize();
            vlk::vec3f normal = model3d.meshes[0].normals[model3d.meshes[0].faces[i].indices[0].normal] * normal_matrix;

            should_cull[i] = dir.dot(normal) >= 0;
        }

        // Local space to homogeneous clip space.
        for (auto& p : positions) {
            p *= mvp;
        }

        // Render mesh.
        for (int i = 0; i < model3d.meshes[0].faces.size(); i++) {
            /*if (should_cull[i]) {
                continue;
            }*/

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
        }

        win.swap_buffers(color_buf);
    }
}