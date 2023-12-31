#include <valkyrie/vlk.hpp>
#include <stdexcept>

// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

using namespace vlk;

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

        vlk::render_rect_color({
            .dst = {{100, 100}, {700, 500}},
              .color = {255, 255, 255, 255},
              .color_buf = color_buf
        });

        vlk::blit_image({
            .dst       = {150,    150                      },
            .src       = {{0, 0}, {ui.width(), ui.height()}},
            .image     = ui,
            .color_buf = color_buf
        });

        win.swap_buffers(color_buf);
    }
}