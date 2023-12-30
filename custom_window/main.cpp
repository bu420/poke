#include <valkyrie/vlk.hpp>
#include <stdexcept>

// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

using namespace vlk;

static void render_rect_color(const rect<size_t> &dst, const color_rgba &color, color_buffer &color_buf) {
    for (size_t x = 0; x < dst.w; ++x) {
        for (size_t y = 0; y < dst.h; ++y) {
            auto &pixel = color_buf.at(dst.x + x, dst.y + y);

            pixel = vlk::default_color_blend(pixel, color);
        }
    }
}

static void blit(size_t dst_x, size_t dst_y, const rect<size_t> &src, const image &image,
                 color_buffer &color_buf) {
    assert(image.channels() == 4);

    for (size_t x = 0; x < src.w; ++x) {
        for (size_t y = 0; y < src.h; ++y) {
            auto &dst_pixel = color_buf.at(dst_x + x, dst_y + y);

            auto src_pixel_iter = image.at(src.x + x, src.y + y);
            auto src_pixel = color_rgba{*(src_pixel_iter + 0), *(src_pixel_iter + 1), *(src_pixel_iter + 2),
                                         *(src_pixel_iter + 3)};

            dst_pixel = vlk::default_color_blend(dst_pixel, src_pixel);
        }
    }
}

int main() {
    vlk::initialize();

    image ui;
    image icon;

    try {
        ui   = vlk::load_image("../assets/windows_ui.png");
        icon = vlk::load_image("../assets/bonzi_buddy.png");
    } catch (std::runtime_error e) {
        std::print("{}\n", e.what());
    }

    size_t screen_width  = GetSystemMetrics(SM_CXSCREEN);
    size_t screen_height = GetSystemMetrics(SM_CYSCREEN);

    color_buffer color_buf{screen_width, screen_height};

    window win{
        {.title       = "",
         .width       = static_cast<i32>(screen_width),
         .height      = static_cast<i32>(screen_height),
         .default_ui  = false,
         .transparent = true}
    };

    win.set_icon(icon);

    while (!win.should_close()) {
        win.poll_events();

        color_buf.clear({0, 0, 0, 0});

        render_rect_color({100, 100, 600, 400}, {255, 255, 255, 255}, color_buf);
        blit(150, 150, rect<size_t>{0, 0, ui.width(), ui.height()}, ui, color_buf);

        win.swap_buffers(color_buf);
    }
}