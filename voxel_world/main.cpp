#include <valkyrie/vlk.hpp>
#include <numbers>

using namespace vlk;

const std::array cube_positions{
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,

    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,

    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,

    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,

    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,

    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,

    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,

    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,

    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,

    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
};

const std::array cube_tex_coords{
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,

    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
};

int main() {
    vlk::initialize();

    const vec2i game_res{128, 128};
    const vec2i window_res{600, 600};

    image atlas;
    sound music;

    try {
        atlas = load_image("../assets/frog.png");
        music = load_sound_wav_pcm_s16le("../assets/drake.wav");
    }
    catch (std::runtime_error e) {
        std::print("{}\n", e.what());
    }

    color_buffer game_color_buf{static_cast<size_t>(game_res.x()), static_cast<size_t>(game_res.y())};
    depth_buffer game_depth_buf{static_cast<size_t>(game_res.x()), static_cast<size_t>(game_res.y())};

    color_buffer window_color_buf{static_cast<size_t>(window_res.x()), static_cast<size_t>(window_res.y())};

    const mat4 projection_matrix = vlk::perspective(static_cast<f32>(game_res.y()) / static_cast<f32>(game_res.x()),
                                                    70 * (static_cast<f32>(std::numbers::pi) / 180),
                                                    0.001f,
                                                    1000.0f);

    const vec3f camera_pos = {-4.0f, 0.0f, -4.0f};

    const mat4 view_matrix = vlk::look_at(camera_pos, 
                                          {0.0f, 0.0f, 0.0f}, 
                                          {0.0f, 1.0f, 0.0f});

    window win{{.title = "voxel world",
                .width = window_res.x(),
                .height = window_res.y()}};

    play_sound(music);

    while (!win.get_should_close()) {
        win.poll_events();

        game_color_buf.clear({255, 255, 255, 255});
        game_depth_buf.clear(1.0f);

        mat4 model_matrix{1.0f};
        model_matrix = model_matrix.rotate_y(static_cast<f32>(get_elapsed_time() * std::numbers::pi / 300));
        model_matrix = model_matrix.rotate_x(static_cast<f32>(get_elapsed_time() * std::numbers::pi / 500));

        const mat4 mvp_matrix = model_matrix * view_matrix * projection_matrix;
        const mat3 normal_matrix = model_matrix.inverse().transpose();

        for (size_t i = 0; i < 12; ++i) {
            const auto &p = cube_positions;
            const auto &t = cube_tex_coords;

            std::array positions{
                vec4f{p[i * 9 + 0], p[i * 9 + 1], p[i * 9 + 2], 1.0f},
                vec4f{p[i * 9 + 3], p[i * 9 + 4], p[i * 9 + 5], 1.0f},
                vec4f{p[i * 9 + 6], p[i * 9 + 7], p[i * 9 + 8], 1.0f}
            };

            for (auto &pos : positions) {
                pos *= mvp_matrix;
            }

            const std::array tex_coords{
                vec2f{t[i * 6 + 0], t[i * 6 + 1]},
                vec2f{t[i * 6 + 2], t[i * 6 + 3]},
                vec2f{t[i * 6 + 4], t[i * 6 + 5]}
            };

            const std::array vertices{
                vertex{positions[0], attribute{tex_coords[0]}},
                vertex{positions[1], attribute{tex_coords[1]}},
                vertex{positions[2], attribute{tex_coords[2]}}
            };

            vlk::render_triangle({.vertices = vertices,
                                  .color_buf = game_color_buf,
                                  .depth_buf = game_depth_buf,
                                  .pixel_shader = [&](const vertex &vertex) {
                                      const vec2f tex_coord = vertex.attributes[0].data.xy();

                                      auto pixel = atlas.sample(tex_coord.x(), tex_coord.y());

                                      return color_rgba{*(pixel + 0),
                                                        *(pixel + 1),
                                                        *(pixel + 2),
                                                        *(pixel + 3)};
                                  }});
        }

        // Draw game color buffer onto window color buffer.
        const std::array vertices{
            vertex{vec4f{-1.0f, -1.0f, 0.0f, 1.0f}, attribute{vec2f{0.0f, 0.0f}}},
            vertex{vec4f{1.0f, -1.0f, 0.0f, 1.0f}, attribute{vec2f{1.0f, 0.0f}}},
            vertex{vec4f{1.0f, 1.0f, 0.0f, 1.0f}, attribute{vec2f{1.0f, 1.0f}}},
            vertex{vec4f{-1.0f, 1.0f, 0.0f, 1.0f}, attribute{vec2f{0.0f, 1.0f}}}
        };

        auto pixel_shader = [&](const vertex &vertex) {
            const vec2f tex_coord = vertex.attributes[0].data.xy();

            color_rgba pixel = game_color_buf.sample(tex_coord.x(), tex_coord.y());

            return pixel;
        };

        vlk::render_triangle({.vertices = {vertices[0], vertices[1], vertices[2]},
                                .color_buf = window_color_buf,
                                .pixel_shader = pixel_shader});

        vlk::render_triangle({.vertices = {vertices[2], vertices[3], vertices[0]},
                                .color_buf = window_color_buf,
                                .pixel_shader = pixel_shader});

        win.swap_buffers(window_color_buf);
    }
}