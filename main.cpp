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

    po::window win("Roterande Kub", W, H);

    po::color_buffer color_buf(W, H);
    po::depth_buffer depth_buf(W, H);

    const po::vertex cube_vertices[36] = {
        po::vertex({-0.5f,-0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,0.f))),
        po::vertex({-0.5f,0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,0.f))),
        po::vertex({0.5f,0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,0.f))),
        po::vertex({-0.5f,-0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,0.f))),
        po::vertex({0.5f,0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,0.f))),
        po::vertex({0.5f,-0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,0.f))),

        po::vertex({0.5f,-0.5f,-0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,0.f))),
        po::vertex({0.5f,0.5f,-0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,0.f))),
        po::vertex({0.5f,0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,0.f))),
        po::vertex({0.5f,-0.5f,-0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,0.f))),
        po::vertex({0.5f,0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,0.f))),
        po::vertex({0.5f,-0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,0.f))),

        po::vertex({0.5f,-0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,0.f,255.f))),
        po::vertex({0.5f,0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,0.f,255.f))),
        po::vertex({-0.5f,0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,0.f,255.f))),
        po::vertex({0.5f,-0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,0.f,255.f))),
        po::vertex({-0.5f,0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,0.f,255.f))),
        po::vertex({-0.5f,-0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,0.f,255.f))),

        po::vertex({-0.5f,-0.5f,0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,255.f))),
        po::vertex({-0.5f,0.5f,0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,255.f))),
        po::vertex({-0.5f,0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,255.f))),
        po::vertex({-0.5f,-0.5f,0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,255.f))),
        po::vertex({-0.5f,0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,255.f))),
        po::vertex({-0.5f,-0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,0.f,255.f))),

        po::vertex({-0.5f,0.5f,-0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,255.f))),
        po::vertex({-0.5f,0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,255.f))),
        po::vertex({0.5f,0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,255.f))),
        po::vertex({-0.5f,0.5f,-0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,255.f))),
        po::vertex({0.5f,0.5f,0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,255.f))),
        po::vertex({0.5f,0.5f,-0.5f,1.f},po::attribute(po::vec3f(0.f,255.f,255.f))),

        po::vertex({0.5f,-0.5f,0.5f,1.f},po::attribute(po::vec3f(255.f,255.f,0.f))),
        po::vertex({-0.5f,-0.5f,0.5f,1.f},po::attribute(po::vec3f(255.f,255.f,0.f))),
        po::vertex({-0.5f,-0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,255.f,0.f))),
        po::vertex({0.5f,-0.5f,0.5f,1.f},po::attribute(po::vec3f(255.f,255.f,0.f))),
        po::vertex({-0.5f,-0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,255.f,0.f))),
        po::vertex({0.5f,-0.5f,-0.5f,1.f},po::attribute(po::vec3f(255.f,255.f,0.f)))
    };

    const po::mat4 projection = po::perspective(
        H / static_cast<float>(W), 
        70 * (static_cast<float>(std::numbers::pi) / 180), 
        .1f, 
        1000.f);

    const po::vec3f camera_pos(1.f, 1.f, 1.f);
    const po::mat4 view = po::look_at(camera_pos, { 0.f, 0.f, 0.f }, { 0.f, -1.f, 0.f });

    while (!win.get_should_close()) {
        win.poll_events();

        color_buf.clear(po::byte3{ 255, 255, 255 });
        depth_buf.clear(1);

        // Model matrix with a rotation animation.
        po::mat4 model(1);
        model = model.rotate_y(static_cast<float>(po::get_elapsed_time() * std::numbers::pi / 2000));
        model = model.rotate_z(static_cast<float>(po::get_elapsed_time() * std::numbers::pi / 4000));

        po::mat4 mvp = model * view * projection;

        // For each of of the cube's triangles.
        for (int i = 0; i < 12; i++) {
            std::array<po::vertex, 3> vertices;

            for (int j = 0; j < 3; j++) {
                vertices[j] = cube_vertices[i * 3 + j];

                // Multiply position with MVP matrix, world space -> clip space.
                vertices[j].position *= mvp;
            }

            po::render_triangle(color_buf, depth_buf, vertices,
                [](const po::vertex& vertex) -> po::byte3 {
                    po::vec4f pos = vertex.position;
                    
                    po::vec3f color(
                        vertex.attributes[0].data[0],
                        vertex.attributes[0].data[1],
                        vertex.attributes[0].data[2]);

                    color *= std::sin(H - (pos.y() / H)) * 1.5f;
                    color += H / 4.f;

                    if (pos.x() > 300) {
                        color.r() = 255;
                    }

                    color *= std::cos(pos.x() / W);

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