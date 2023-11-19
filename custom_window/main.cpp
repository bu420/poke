#include <valkyrie/vlk.math.hpp>
#include <valkyrie/vlk.gfx.hpp>
#include <valkyrie/vlk.system.hpp>

using namespace vlk;

int main() {
    constexpr size_t width = 300;
    constexpr size_t height = 300;

    initialize();

    window win{{.title = "custom window",
                .width = width,
                .height = height,
                .default_ui = true,
                .transparent = true}};

    color_buffer color_buf{width, height};

    while (!win.get_should_close()) {
        win.poll_events();

        std::array vertices{
            vertex{{-0.5f, -0.5f, 0.0f, 1.0f}},
            vertex{{0.5f, -0.5f, 0.0f, 1.0f}},
            vertex{{0.0f, 0.5f, 0.0f, 1.0f}}
        };

        /*const auto transformation_matrix =
            mat4{1.0f}.rotate_z(static_cast<f32>(std::sin(get_elapsed_time() / 500) * 8));

        for (auto& vertex : vertices) {
            vertex.pos *= transformation_matrix;
        }*/

        color_buf.clear({0, 0, 0, 0});

        render_triangle({.vertices = vertices,
                         .color_buf = color_buf,
                         .pixel_shader = [](const vertex&) {
                             return color_rgba{255, 0, 0, 255};
                         }});

        win.swap_buffers(color_buf);
    }
}