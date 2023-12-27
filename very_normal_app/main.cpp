#include <valkyrie/vlk.hpp>
#include <stdexcept>

using namespace vlk;

int main() {
    const size_t width = 400;
    const size_t height = 300;

    vlk::initialize();

    window win{{.title = "",
                .width = width,
                .height = height,
                .default_ui = false,
                .transparent = true}};

    color_buffer color_buf{width, height};

    image image;
    try {
        image = vlk::load_image("../assets/frog_with_margin.png");
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

        vlk::render_triangle({.vertices = vertices_0,
                         .color_buf = color_buf,
                         .pixel_shader = pixel_shader});

        vlk::render_triangle({.vertices = vertices_1,
                         .color_buf = color_buf,
                         .pixel_shader = pixel_shader});

        win.swap_buffers(color_buf);
    }
}