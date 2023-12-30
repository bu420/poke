#include <valkyrie/vlk.hpp>
#include <stdexcept>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

using namespace vlk;

static void render_quad(const rect<size_t> &dst, const rect<size_t> &src, const image &image,
                        color_buffer &color_buf) {
    /*std::array vertices{
        vertex{{1.0f, 1.0f, 0.0f, 1.0f}, attribute{vec2f{1.0f, 1.0f}}},
        vertex{{1.0f, -1.0f, 0.0f, 1.0f}, attribute{vec2f{1.0f, 0.0f}}},
        vertex{{-1.0f, -1.0f, 0.0f, 1.0f}, attribute{vec2f{0.0f, 0.0f}}},
        vertex{{-1.0f, 1.0f, 0.0f, 1.0f}, attribute{vec2f{0.0f, 1.0f}}}
    };

    auto pixel_shader = [&](const vertex &vertex) {
        auto sample = image.sample(vertex.attributes[0].data[0],
                                   vertex.attributes[0].data[1]);

        return color_rgba{*(sample + 0),
                          *(sample + 1),
                          *(sample + 2),
                          *(sample + 3)};
    };

    vlk::render_triangle({.vertices     = vertices_0,
                          .color_buf    = color_buf,
                          .pixel_shader = pixel_shader});

    vlk::render_triangle({.vertices     = vertices_1,
                          .color_buf    = color_buf,
                          .pixel_shader = pixel_shader});*/
}

static void blit(size_t dst_x, size_t dst_y, const rect<size_t> &src, const image &image,
                 color_buffer &color_buf) {
    assert(image.channels == 4);

    for (size_t x = 0; x <= src.w; ++x) {
        for (size_t y = 0; y <= src.h; ++y) {
            auto src_pixel = image.at(src.x + x, src.y + y);

            color_buf.at(dst_x + x, dst_y + y) =
                color_rgba{*(src_pixel + 0), *(src_pixel + 1), *(src_pixel + 2), *(src_pixel + 3)};
        }
    }
}

int main() {
    vlk::initialize();

    image ui_image;

    try {
        ui_image = vlk::load_image("../assets/default_window.png");
    } catch (std::runtime_error e) {
        std::print("{}\n", e.what());
    }

    size_t screen_width  = GetSystemMetrics(SM_CXSCREEN);
    size_t screen_height = GetSystemMetrics(SM_CYSCREEN);

    color_buffer color_buf{screen_width, screen_height};

    window win{
        {.title       = "Hello World",
         .width       = static_cast<i32>(screen_width),
         .height      = static_cast<i32>(screen_height),
         .default_ui  = false,
         .transparent = true}
    };

    while (!win.should_close()) {
        win.poll_events();

        color_buf.clear({0, 0, 0, 0});

        blit(100, 100, {0, 0, ui_image.width(), ui_image.height()}, ui_image, color_buf);

        win.swap_buffers(color_buf);
    }
}