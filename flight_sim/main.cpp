#include <valkyrie/vlk.math.hpp>
#include <valkyrie/vlk.gfx.hpp>
#include <valkyrie/vlk.system.hpp>
#include <valkyrie/vlk.ext.parse_obj.hpp>
#include <numbers>
#include <algorithm>
#include <print>
#include <filesystem>

using namespace vlk;

int main() {
    const size_t width{600};
    const size_t height{600};

    initialize();

    model model;
    try {
        model = parse_obj("../assets/gear/Gear1.obj");
    }
    catch (std::runtime_error e) {
        std::print("{}\n", e.what());
        return -1;
    }

    std::print("Assets loaded.\n");

    window win{"flight sim", width, height};

    color_buffer color_buf{width, height};
    depth_buffer depth_buf{width, height};

    const mat4 projection_matrix = perspective(height / static_cast<f32>(width),
                                               70 * (static_cast<f32>(std::numbers::pi) / 180),
                                               0.001f,
                                               1000.f);

    const vec3f camera_pos{0.f, 10.f, -15.f};
    const mat4 view_matrix{look_at(camera_pos, {0.f, 10.f, 0.f}, {0.f, -1.f, 0.f})};

    while (!win.get_should_close()) {
        win.poll_events();

        color_buf.clear({255, 255, 255});
        depth_buf.clear(1.0f);

        mat4 model_matrix{1.0f};
        model_matrix = model_matrix.rotate_y(static_cast<f32>(get_elapsed_time() * std::numbers::pi / 1500));
        model_matrix = model_matrix.translate({0.0f, 5.0f, 0.0f});

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