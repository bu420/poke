#include <valkyrie/vlk.math.hpp>
#include <valkyrie/vlk.gfx.hpp>
#include <valkyrie/vlk.system.hpp>
#include <stdexcept>
#include <print>

using namespace vlk;

int main() {
    constexpr size_t width = 400;
    constexpr size_t height = 300;

    initialize();

    window win{{.title = "custom window",
                .width = width,
                .height = height,
                .default_ui = false,
                .transparent = true}};

    color_buffer color_buf{width, height};

    image image;
    try {
        image = load_image("..\\assets\\frog_with_margin.png");
    }
    catch (std::runtime_error e) {
        std::print("{}\n", e.what());
    }

    while (!win.get_should_close()) {
        win.poll_events();

        std::array vertices_0{
            vertex{{1.0f, 1.0f, 0.0f, 1.0f}, attribute{vec2f{1.0f, 1.0f}}},
            vertex{{1.0f, -1.0f, 0.0f, 1.0f}, attribute{vec2f{1.0f, 0.0f}}},
            vertex{{-1.0f, -1.0f, 0.0f, 1.0f}, attribute{vec2f{0.0f, 0.0f}}}
        };

        std::array vertices_1{
            vertex{{-1.0f, -1.0f, 0.0f, 1.0f}, attribute{vec2f{0.0f, 0.0f}}},
            vertex{{-1.0f, 1.0f, 0.0f, 1.0f}, attribute{vec2f{0.0f, 1.0f}}},
            vertex{{1.0f, 1.0f, 0.0f, 1.0f}, attribute{vec2f{1.0f, 1.0f}}}
        };

        color_buf.clear({0, 0, 0, 0});

        auto pixel_shader = [&](const vertex &vertex) {
            auto sample = image.sample(vertex.attributes[0].data[0],
                                       vertex.attributes[0].data[1]);

            return color_rgba{*sample,
                              *(sample + 1),
                              *(sample + 2),
                              *(sample + 3)};
        };

        render_triangle({.vertices = vertices_0,
                         .color_buf = color_buf,
                         .pixel_shader = pixel_shader});

        render_triangle({.vertices = vertices_1,
                         .color_buf = color_buf,
                         .pixel_shader = pixel_shader});

        win.swap_buffers(color_buf);
    }
}