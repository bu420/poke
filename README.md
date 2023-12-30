# Valkyrie

## About
A Win32 game framework. 

## Example
```cpp
#include <valkyrie/vlk.hpp>
#include <numbers>

using namespace vlk;

int main() {
    const size_t width  = 600;
    const size_t height = 400;

    vlk::initialize();

    model model;
    sound music;
    sound boom;
    image icon;

    std::print("Loading assets...\n");

    try {
        model = vlk::load_obj("../assets/lexus/lexus.obj");
        music = vlk::load_sound_wav_pcm_s16le("../assets/drake.wav");
        boom  = vlk::load_sound_wav_pcm_s16le("../assets/vine_boom.wav");
        icon  = vlk::load_image("../assets/runescape.ico");
    } catch (std::runtime_error e) {
        std::print("{}\n", e.what());
        return -1;
    }

    std::print("Done.\n");

    window win{
        {.title = "", .width = width, .height = height, .transparent = true}
    };

    win.set_icon(icon);

    color_buffer color_buf{width, height};
    depth_buffer depth_buf{width, height};

    const mat4 projection_matrix = vlk::perspective(
        static_cast<f32>(width) / height, 70 * static_cast<f32>(std::numbers::pi) / 180, 0.001f, 1000.0f);

    const vec3f camera_pos{100.0f, 30.0f, 0.0f};
    const mat4 view_matrix = vlk::look_at(camera_pos, {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f});

    vlk::play_sound(boom);
    vlk::play_sound(music, true);

    while (!win.should_close()) {
        win.poll_events();

        color_buf.clear({0, 0, 0, 0});
        depth_buf.clear(1.0f);

        mat4 model_matrix{1.0f};
        model_matrix = model_matrix.rotate_y(static_cast<f32>(get_elapsed_time() * std::numbers::pi / 700));

        mat4 mvp_matrix    = model_matrix * view_matrix * projection_matrix;
        mat3 normal_matrix = model_matrix.inverse().transpose();

        vlk::render_model({.model         = model,
                           .mvp_matrix    = mvp_matrix,
                           .normal_matrix = normal_matrix,
                           .color_buf     = color_buf,
                           .depth_buf     = depth_buf});

        win.swap_buffers(color_buf);
    }
}
```
