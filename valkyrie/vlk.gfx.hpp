#pragma once

#include <vector>
#include <array>
#include <optional>
#include <functional>
#include <string>
#include <unordered_map>

#include "vlk.types.hpp"
#include "vlk.math.hpp"

namespace vlk {
    template <typename T> 
    class buffer2d {
    public:
        buffer2d(size_t width, size_t height) :
            width{width},
            height{height} {
            data.resize(width * height);
        }

        void clear(T value) {
            std::fill(data.begin(), data.end(), value);
        }

        size_t get_width() const { return width; }
        size_t get_height() const { return height; }

        T& operator [] (size_t i) { return data.at(i); }
        const T& operator [] (size_t i) const { return data.at(i); }

        T& at(size_t x, size_t y) { return data.at(y * width + x); }
        const T& at(size_t x, size_t y) const { return data.at(y * width + x); }

    protected:
        size_t width;
        size_t height;
        std::vector<T> data;
    };

    using color_buffer = buffer2d<byte3>;
    using depth_buffer = buffer2d<float>;

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
        vec4f position;

        std::array<attribute, 4> attributes;
        u8 attribute_count;

        vertex() : attribute_count{0} {}
        vertex(const vec4f& position) : position{position}, attribute_count{0} {}

        template <typename... T> 
        vertex(const vec4f& position, T... attributes) :
            position{position},
            attributes{attributes...},
            attribute_count{sizeof...(T)} {
        }

        vertex lerp(const vertex& other, f32 amount) const;
    };

    using pixel_shader_callback =
        std::function<std::optional<byte3>(const vertex&)>;

    void render_triangle(std::array<vertex, 3> vertices, 
                         optional_ref<color_buffer> color_buf,
                         optional_ref<depth_buffer> depth_buf,
                         pixel_shader_callback pixel_shader);

    struct image {
        size_t width;
        size_t height;
        u8 channels;
        std::vector<std::byte> data;
    };
}