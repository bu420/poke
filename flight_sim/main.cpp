#include <valkyrie/vlk.hpp>
#include <numbers>
#include <algorithm>
#include <filesystem>

using namespace vlk;

int main() {
    const size_t width = 600;
    const size_t height = 600;

    initialize();

    std::print("Loading assets...\n");

    model model;
    sound windows95;
    sound music;

    try {
        model = parse_obj("../assets/lexus/lexus.obj");
        windows95 = load_sound_wav_pcm_s16le("../assets/Windows-95-startup-sound.wav");
        music = load_sound_wav_pcm_s16le("../assets/dnb.wav");
    }
    catch (std::runtime_error e) {
        std::print("{}\n", e.what());
        return -1;
    }

    std::print("Done.\n");

    window win{{.title = "custom window",
                .width = width,
                .height = height}};

    color_buffer color_buf{width, height};
    depth_buffer depth_buf{width, height};

    const mat4 projection_matrix = perspective(height / static_cast<f32>(width),
                                               70 * (static_cast<f32>(std::numbers::pi) / 180),
                                               0.001f,
                                               1000.0f);

    const vec3f camera_pos{100.0f, 30.0f, 0.0f};
    const mat4 view_matrix{look_at(camera_pos, {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f})};

    music.play();

    auto start = get_elapsed_time();
    bool played = false;

    while (!win.get_should_close()) {
        win.poll_events();

        if (not played && (get_elapsed_time() - start > 10000)) {
            windows95.play();
            played = true;
        }

        color_buf.clear({255, 255, 255});
        depth_buf.clear(1.0f);

        mat4 model_matrix{1.0f};
        model_matrix = model_matrix.rotate_y(static_cast<f32>(get_elapsed_time() * std::numbers::pi / 700));

        mat4 mvp_matrix{model_matrix * view_matrix * projection_matrix};
        mat3 normal_matrix{model_matrix.inverse().transpose()};

        render_model(
            {
                .model{model},
                .mvp_matrix{mvp_matrix},
                .normal_matrix{normal_matrix},
                .color_buf{color_buf},
                .depth_buf{depth_buf}
            });

        win.swap_buffers(color_buf);
    }
}