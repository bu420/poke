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
        std::byte r;
        std::byte g;
        std::byte b;
    };

    struct color_rgba {
        std::byte r;
        std::byte g;
        std::byte b;
        std::byte a;
    };

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

    protected:
        size_t width;
        size_t height;
        std::vector<T> data;
    };

    using color_buffer = buffer<color_rgb>;
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

    using pixel_shader_callback = std::function<color_rgba(const vertex& vertex,
                                                           optional_ref<std::any> user_data)>;

    using color_blend_func_callback = std::function<color_rgb(const color_rgb& old_color,
                                                              const color_rgba& new_color)>;

    color_rgb default_color_blend_func(const color_rgb& old_color,
                                       const color_rgba& new_color);

    struct render_triangle_params {
        std::array<vertex, 3> vertices;
        optional_ref<std::any> user_data;
        optional_ref<color_buffer> color_buf;
        optional_ref<depth_buffer> depth_buf;
        pixel_shader_callback pixel_shader;
        color_blend_func_callback color_blend_func = default_color_blend_func;
    };

    void render_triangle(const render_triangle_params& params);

    struct image {
        size_t width;
        size_t height;
        u8 channels;
        std::vector<std::byte> data;
    };
}