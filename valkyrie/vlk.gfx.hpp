#pragma once

#include <vector>
#include <array>
#include <string>
#include <optional>
#include <functional>
#include <any>
#include <unordered_map>

#include "vlk.types.hpp"
#include "vlk.math.hpp"

namespace vlk {
    struct color_rgb {
        u8 r;
        u8 g;
        u8 b;
    };

    struct color_rgba {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };

    static_assert(sizeof(color_rgb) == 3 and sizeof(color_rgba) == 4);

    template <typename T>
    class buffer {
    public:
        buffer(size_t width, size_t height) :
            width{width},
            height{height} {
            data.resize(width * height);
        }

        void clear(T value) {
            std::fill(data.begin(), data.end(), value);
        }

        size_t get_width() const { return width; }
        size_t get_height() const { return height; }

        T& operator [] (size_t i) {
            assert(i >= 0 and i < data.size());
            return data[i];
        }
        const T& operator [] (size_t i) const {
            assert(i >= 0 and i < data.size());
            return data[i];
        }

        T& at(size_t x, size_t y) {
            return (*this)[y * width + x];
        }
        const T& at(size_t x, size_t y) const {
            return (*this)[y * width + x];
        }

        T sample(f32 x, f32 y) const {
            return at(static_cast<size_t>(std::round(x * static_cast<f32>(width - 1))),
                      static_cast<size_t>(std::round(y * static_cast<f32>(height - 1))));
        }

    protected:
        size_t width;
        size_t height;
        std::vector<T> data;
    };

    using color_buffer = buffer<color_rgba>;
    using depth_buffer = buffer<f32>;

    struct attribute {
        vec4f data;
        u8 size;

        attribute() : size{0} {}
        attribute(f32 value) : data{value, 0.f, 0.f, 0.f}, size{1} {}
        attribute(const vec2f& value) : data{value.x(), value.y(), 0.f, 0.f}, size{2} {}
        attribute(const vec3f& value) : data{value.x(), value.y(), value.z(), 0.f}, size{3} {}
        attribute(const vec4f& value) : data{value.x(), value.y(), value.z(), value.w()}, size{4} {}

        attribute lerp(const attribute& other, f32 amount) const;

        void operator += (const attribute& a);
    };

    struct vertex {
        vec4f pos;

        std::array<attribute, 4> attributes;
        u8 attribute_count;

        vertex() = default;
        vertex(const vec4f& pos) : pos{pos}, attribute_count{0} {}

        template <typename... T>
        vertex(const vec4f& pos, T... attributes) :
            pos{pos},
            attributes{attributes...},
            attribute_count{sizeof...(T)} {
        }

        vertex lerp(const vertex& other, f32 amount) const;
    };

    struct line3d {
        vertex start;
        vertex end;
    };

    struct line3d_stepper {
        enum class calc_steps_based_on {
            largest_difference,
            x_difference,
            y_difference
        };

        vertex current;
        vertex increment;

        i32 steps{0};
        i32 i{0};

        line3d_stepper(line3d line, calc_steps_based_on line_type);

        bool step();
    };

    using pixel_shader_func = std::function<color_rgba(const vertex&)>;

    using color_blend_func = std::function<color_rgba(const color_rgba& old_color,
                                                      const color_rgba& new_color)>;

    color_rgba default_color_blend(const color_rgba& old_color,
                                   const color_rgba& new_color);

    struct render_triangle_params {
        std::array<vertex, 3> vertices;
        optional_ref<color_buffer> color_buf;
        optional_ref<depth_buffer> depth_buf;
        pixel_shader_func pixel_shader;
        color_blend_func color_blend = default_color_blend;
    };

    void render_triangle(const render_triangle_params& params);

    struct image {
        size_t width;
        size_t height;
        size_t channels;
        std::vector<u8> data;

        std::vector<u8>::iterator at(size_t x, size_t y);
        std::vector<u8>::const_iterator at(size_t x, size_t y) const;

        std::vector<u8>::iterator sample(f32 x, f32 y);
        std::vector<u8>::const_iterator sample(f32 x, f32 y) const;

        static image flip_vertically(const image &image);
    };

    struct model {
        struct material {
            std::string name;

            size_t albedo_map_index;
            size_t normal_map_index;
            size_t roughness_map_index;
            size_t metallic_map_index;
            size_t opacity_map_index;
        };

        struct mesh {
            size_t material_index;

            bool has_tex_coords;
            bool has_normals;

            struct face {
                std::array<size_t, 3> position_indices;
                std::array<size_t, 3> tex_coord_indices;
                std::array<size_t, 3> normal_indices;
            };

            std::vector<face> faces;
        };

        std::vector<vec3f> positions;
        std::vector<vec2f> tex_coords;
        std::vector<vec3f> normals;

        std::vector<mesh> meshes;
        std::vector<material> materials;
        std::vector<image> images;

        static constexpr size_t no_index = static_cast<size_t>(-1);
    };

    using model_pixel_shader_func = std::function<color_rgba(const vertex&,
                                                             const model&,
                                                             size_t material_index)>;

    color_rgba default_model_pixel_shader(const vertex& vertex,
                                          const model& model,
                                          size_t material_index);

    struct render_model_params {
        model model;
        mat4 mvp_matrix;
        mat3 normal_matrix;

        std::array<vertex, 3> vertices;
        optional_ref<color_buffer> color_buf;
        optional_ref<depth_buffer> depth_buf;
        model_pixel_shader_func pixel_shader = default_model_pixel_shader;
        color_blend_func color_blend = default_color_blend;
    };

    void render_model(const render_model_params& params);
}