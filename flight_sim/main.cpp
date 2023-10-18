#include <valkyrie/vlk.math.hpp>
#include <valkyrie/vlk.gfx.hpp>
#include <valkyrie/vlk.system.hpp>
#include <valkyrie/vlk.ext.model.hpp>

#include <numbers>
#include <algorithm>
#include <iostream>

using namespace vlk;

int main() {
    const size_t width{600};
    const size_t height{600};

    initialize();

    model model;
    try {
        model = parse_obj("../assets/bullfrog.obj");
    }
    catch (std::runtime_error e) {
        std::cout << e.what() << std::endl;
    }

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

        color_buf.clear({std::byte{255}, std::byte{255}, std::byte{255}});
        depth_buf.clear(1.0f);

        mat4 model_matrix{1.0f};
        model_matrix = model_matrix.rotate_x(static_cast<f32>(90.f * (std::numbers::pi / 180)));
        model_matrix = model_matrix.rotate_y(static_cast<f32>(get_elapsed_time() * std::numbers::pi / 1500));
        model_matrix = model_matrix.translate({0.f, 0.f, static_cast<f32>((std::sin(get_elapsed_time() / 750) + 1) * 17)});

        mat4 mvp_matrix{model_matrix * view_matrix * projection_matrix};
        mat3 normal_matrix{model_matrix.inverse().transpose()};

        render_model(
            {
                .model{model},
                .mvp_matrix{mvp_matrix},
                .normal_matrix{normal_matrix},
                .color_buf{color_buf},
                .depth_buf{depth_buf},
            });

        /*render_triangle(
            {
                .vertices{
                    vertex{vec4f{-0.5f, -0.5f, 0.0f, 1.0f}},
                    vertex{vec4f{0.5f, -0.5f, 0.0f, 1.0f}},
                    vertex{vec4f{0.0f, 0.5f, 0.0f, 1.0f}}
                },
                .color_buf{color_buf},
                .depth_buf{depth_buf},
                .pixel_shader{
                    [](const vertex&, optional_ref<std::any>) {
                        return color_rgba{255_byte, 0_byte, 0_byte};
                    }
                }
            });*/

        win.swap_buffers(color_buf);
    }
}