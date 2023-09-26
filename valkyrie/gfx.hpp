#pragma once

#include <vector>
#include <array>
#include <optional>
#include <functional>
#include <string>

#include "util.hpp"
#include "math.hpp"

namespace vlk {
    template <typename T> class buffer2d {
    public:
        buffer2d(std::size_t width, std::size_t height) :
            width{width},
            height{height} {
            data.resize(width * height);
        }

        void clear(T value) {
            std::fill(data.begin(), data.end(), value);
        }

        std::size_t get_width() const { return width; }
        std::size_t get_height() const { return height; }

        T& operator [] (std::size_t i) { return data.at(i); }
        const T& operator [] (std::size_t i) const { return data.at(i); }

        T& at(std::size_t x, std::size_t y) { return data.at(y * width + x); }
        const T& at(std::size_t x, std::size_t y) const { return data.at(y * width + x); }

    protected:
        std::size_t width;
        std::size_t height;
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

        template <typename... T> vertex(const vec4f& position, T... attributes) :
            position{position},
            attributes{attributes...},
            attribute_count{sizeof...(T)} {
        }

        vertex lerp(const vertex& other, f32 amount) const;
    };

    using pixel_shader_callback =
        std::function<std::optional<byte3>(const vertex&)>;

    void render_triangle(optional_reference<color_buffer> color_buf,
                         optional_reference<depth_buffer> depth_buf,
                         std::array<vertex, 3> vertices,
                         pixel_shader_callback pixel_shader);

    struct image {
        std::size_t width;
        std::size_t height;
        u8 channels;
        std::vector<std::byte> data;
    };

    struct material {
        std::optional<std::size_t> albedo_map_image_index;
        std::optional<std::size_t> normal_map_image_index;
        std::optional<std::size_t> roughness_map_image_index;
        std::optional<std::size_t> metallic_map_image_index;
        std::optional<std::size_t> opacity_map_image_index;
    };

    struct model {
        struct face {
            std::array<std::size_t, 3> position_indices;
            std::optional<std::array<std::size_t, 3>> tex_coord_indices;
            std::optional<std::array<std::size_t, 3>> normal_indices;
            std::optional<std::size_t> material_index;
        };

        std::vector<vec3f> positions;
        std::vector<vec2f> tex_coords;
        std::vector<vec3f> normals;
        std::vector<material> materials;
        std::vector<image> images;
        std::vector<face> faces;
    };

    std::optional<model> load_obj_model(const std::string& path);

    /*class vmod_parser {
    public:
        static std::optional<model> parse(const std::vector<std::byte>& buf);
        static std::optional<model> parse(const std::string& path);

    private:
        static i32 get_next_int(const std::vector<std::byte>& buf, 
                                std::size_t& byte_pos);
        static f32 get_next_float(const std::vector<std::byte>& buf, 
                                  std::size_t& byte_pos);
    };*/
}