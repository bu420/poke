#include "math.hpp"
#include "gfx.hpp"
#include "platform.hpp"

#include <numbers>
#include <algorithm>
#include <iostream>

#define W 600
#define H 600

int main() {
    namespace po = poke;

    po::initialize();

    po::mesh mesh;
    std::cout << "Loading assets..." << std::endl;
    if (!mesh.load_obj("assets/bullfrog.obj")) {
        std::cout << "Failed to load assets." << std::endl;
        return -1;
    }
    std::cout << "Done." << std::endl;

    po::window win("poke", W, H);

    po::color_buffer color_buf(W, H);
    po::depth_buffer depth_buf(W, H);

    const po::mat4 projection = po::perspective(
        H / static_cast<float>(W), 
        70 * (static_cast<float>(std::numbers::pi) / 180), 
        20.f, 
        1000.f);

    const po::vec3f camera_pos(0.f, 10.f, -15.f);
    const po::mat4 view = po::look_at(camera_pos, { 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f });

    while (!win.get_should_close()) {
        win.poll_events();

        color_buf.clear({ 255, 255, 255 });
        depth_buf.clear(1.0f);

        po::mat4 model(1);
        model = model.rotate_x(static_cast<float>(90.f * (std::numbers::pi / 180)));
        model = model.rotate_y(static_cast<float>(po::get_elapsed_time() * std::numbers::pi / 1500));
        //model = model.rotate_z(static_cast<float>(po::get_elapsed_time() * std::numbers::pi / 4000));
        model = model.translate({ 0.f, 0.f, static_cast<float>((std::sin(po::get_elapsed_time() / 750) + 1) * 17) });

        po::mat4 mvp = model * view * projection;
        po::mat3 normal_matrix = model.inverse().transpose();

        // Make a copy that we can mutate.
        std::vector<po::vec4f> positions(mesh.positions.size());
        for (int i = 0; i < mesh.positions.size(); i++) {
            positions[i].x() = mesh.positions[i].x();
            positions[i].y() = mesh.positions[i].y();
            positions[i].z() = mesh.positions[i].z();
            positions[i].w() = 1.f;
        }

        std::vector<bool> should_cull(mesh.faces.size());

        // Backface culling.
        for (int i = 0; i < mesh.faces.size(); i++) {
            // Pick any one of the triangle's points, put it in model space and check if the triangle should be culled.
            po::vec4f any_point_on_triangle = positions[mesh.faces[i].indices[0].pos] * model;

            po::vec3f dir = (any_point_on_triangle.xyz() - camera_pos).normalize();
            po::vec3f normal = mesh.normals[mesh.faces[i].indices[0].normal] * normal_matrix;

            should_cull[i] = dir.dot(normal) >= 0;
        }

        // Local space to homogeneous clip space.
        for (auto& p : positions) {
            p *= mvp;
        }

        // Render mesh.
        for (int i = 0; i < mesh.faces.size(); i++) {
            /*if (should_cull[i]) {
                continue;
            }*/

            const auto& face = mesh.faces[i];

            std::array<po::vec3f, 3> normals = {
                mesh.normals[face.indices[0].normal] * normal_matrix,
                mesh.normals[face.indices[1].normal] * normal_matrix,
                mesh.normals[face.indices[2].normal] * normal_matrix
            };

            std::array<po::vertex, 3> vertices = {
                po::vertex(positions[face.indices[0].pos], po::attribute(normals[0])),
                po::vertex(positions[face.indices[1].pos], po::attribute(normals[1])),
                po::vertex(positions[face.indices[2].pos], po::attribute(normals[2]))
            };

            po::render_triangle(color_buf, depth_buf, vertices,
                [](const po::vertex& vertex) -> po::byte3 {
                    po::vec4f pos = vertex.position;

                    po::vec3f normal(vertex.attributes[0].data.xyz());
                    po::vec3f color((normal + 1.f) * 127.f);

                    return {
                        static_cast<unsigned char>(color.x()),
                        static_cast<unsigned char>(color.y()),
                        static_cast<unsigned char>(color.z())
                    };
                });
        }

        win.swap_buffers(color_buf);
    }
}