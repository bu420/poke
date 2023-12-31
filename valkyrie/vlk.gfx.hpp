#pragma once

#include <vector>
#include <array>
#include <string>
#include <functional>

#include "vlk.types.hpp"
#include "vlk.math.hpp"

#ifndef VLK_MAX_ATTRIBUTES
#define VLK_MAX_ATTRIBUTES 4
#endif

namespace vlk {
    using color_rgb  = vec3<u8>;
    using color_rgba = vec4<u8>;

    static_assert(sizeof(color_rgb) == 3 && sizeof(color_rgba) == 4);

    template <typename T>
    class buffer {
    public:
        buffer(size_t width, size_t height) : m_width{width}, m_height{height} {
            m_data.resize(width * height);
        }

        void clear(T value) { std::fill(m_data.begin(), m_data.end(), value); }

        size_t width() const { return m_width; }
        size_t height() const { return m_height; }

        T &operator[](size_t i) {
            VLK_ASSERT_FAST(i >= 0 && i < m_data.size(), "Out of bounds.");
            return m_data[i];
        }
        const T &operator[](size_t i) const {
            VLK_ASSERT_FAST(i >= 0 && i < m_data.size(), "Out of bounds.");
            return m_data[i];
        }

        T &at(size_t x, size_t y) { return (*this)[y * width() + x]; }
        const T &at(size_t x, size_t y) const { return (*this)[y * width() + x]; }

        T sample(f32 x, f32 y) const {
            return at(static_cast<size_t>(std::round(x * static_cast<f32>(width() - 1))),
                      static_cast<size_t>(std::round(y * static_cast<f32>(height() - 1))));
        }

    protected:
        size_t m_width;
        size_t m_height;
        std::vector<T> m_data;
    };

    using color_buffer = buffer<color_rgba>;
    using depth_buffer = buffer<f32>;

    class attrib {
    public:
        vec4f data;
        u32 size;

        attrib() : data{}, size{0} {}
        attrib(f32 value) : data{value, 0.f, 0.f, 0.f}, size{1} {}
        attrib(const vec2f &value) : data{value, 0.f, 0.f}, size{2} {}
        attrib(const vec3f &value) : data{value, 0.f}, size{3} {}
        attrib(const vec4f &value) : data{value}, size{4} {}
        attrib(const vec4f &value, u32 size) : data{value}, size{size} {}

        attrib lerp(const attrib &other, f32 amount) const;

        f32 &operator[](u32 i) { return data[i]; };
        f32 operator[](u32 i) const { return data[i]; };

        void operator+=(const attrib &a);
    };

    class vertex {
    public:
        vec4f pos;
        u32 count;

        vertex() = default;
        vertex(const vec4f &pos) : pos{pos}, count{0} {}
        vertex(const vec4f &pos, u32 count) : pos{pos}, count{count} {}

        template <typename... T>
        vertex(const vec4f &pos, T &&...attribs) : pos{pos}, m_attribs{attribs...}, count{sizeof...(T)} {}

        vertex lerp(const vertex &other, f32 amount) const;

        attrib &operator[](size_t i) {
            VLK_ASSERT_FAST(i >= 0 && i < count, "Out of bounds.");
            return m_attribs[i];
        }
        const attrib &operator[](size_t i) const {
            VLK_ASSERT_FAST(i >= 0 && i < count, "Out of bounds.");
            return m_attribs[i];
        }

    private:
        std::array<attrib, VLK_MAX_ATTRIBUTES> m_attribs;
    };

    struct line {
        vertex start;
        vertex end;
    };

    class line_stepper {
    public:
        enum class calc_steps_based_on {
            largest_diff,
            x_diff,
            y_diff,
        };

        line_stepper(line line, calc_steps_based_on line_type = calc_steps_based_on::largest_diff);

        const vertex &vertex() const { return m_current; }

        bool step();

    private:
        vlk::vertex m_current;
        vlk::vertex m_increment;

        u32 m_steps;
        u32 m_i;
    };

    using pixel_shader_func = std::function<color_rgba(const vertex &)>;

    using color_blend_func =
        std::function<color_rgba(const color_rgba &old_color, const color_rgba &new_color)>;

    color_rgba default_color_blend(const color_rgba &old_color, const color_rgba &new_color);

    struct render_triangle_params {
        std::array<vertex, 3> vertices;
        optional_ref<color_buffer> color_buf;
        optional_ref<depth_buffer> depth_buf;
        pixel_shader_func pixel_shader;
        color_blend_func color_blend = default_color_blend;
    };

    void render_triangle(const render_triangle_params &params);

    struct render_rect_color_params {
        rect<size_t> dst;
        color_rgba color;
        color_buffer &color_buf;
    };

    void render_rect_color(const render_rect_color_params &params);

    struct render_rect_color_rounded_params : render_rect_color_params {
        f32 radius        = 0;
        bool top_left     = true;
        bool top_right    = true;
        bool bottom_left  = true;
        bool bottom_right = true;
    };

    void render_rect_color_rounded(const render_rect_color_rounded_params &params);

    class image {
    public:
        image() = default;
        image(size_t width, size_t height, size_t channels);

        size_t width() const { return m_width; }
        size_t height() const { return m_height; }
        size_t channels() const { return m_channels; }

        std::vector<u8>::iterator at(size_t x, size_t y);
        std::vector<u8>::const_iterator at(size_t x, size_t y) const;

        std::vector<u8>::iterator sample(f32 x, f32 y);
        std::vector<u8>::const_iterator sample(f32 x, f32 y) const;

        static color_rgb to_rgb(std::vector<u8>::const_iterator iter);
        static color_rgba to_rgba(std::vector<u8>::const_iterator iter);

        static image flip_vertically(const image &image);

    private:
        size_t m_width;
        size_t m_height;
        size_t m_channels;

        std::vector<u8> m_data;
    };

    struct blit_image_params {
        vec2<size_t> dst;
        rect<size_t> src;
        const image &image;
        color_buffer &color_buf;
    };

    void blit_image(const blit_image_params &params);

    struct model {
        struct material {
            std::string name;

            // Indices into model::images.
            size_t albedo;
            size_t normal;
            size_t roughness;
            size_t metallic;
            size_t opacity;
        };

        struct mesh {
            size_t material_index;

            bool has_tex_coords;
            bool has_normals;

            struct face {
                std::array<size_t, 3> positions;   // Indices into model::positions.
                std::array<size_t, 3> tex_coords;  // Indices into model::tex_coords.
                std::array<size_t, 3> normals;     // Indicies into model::normals.
            };

            std::vector<face> faces;
        };

        std::vector<vec3f> positions;
        std::vector<vec2f> tex_coords;
        std::vector<vec3f> normals;

        std::vector<mesh> meshes;
        std::vector<material> materials;
        std::vector<image> images;

        static constexpr size_t null_index = static_cast<size_t>(-1);
    };

    using model_pixel_shader_func =
        std::function<color_rgba(const vertex &, const model &, size_t material_index)>;

    color_rgba default_model_pixel_shader(const vertex &vertex, const model &model, size_t material_index);

    struct render_model_params {
        model model;
        mat4 mvp_matrix;
        mat3 normal_matrix;

        std::array<vertex, 3> vertices;
        optional_ref<color_buffer> color_buf;
        optional_ref<depth_buffer> depth_buf;
        model_pixel_shader_func pixel_shader = default_model_pixel_shader;
        color_blend_func color_blend         = default_color_blend;
    };

    void render_model(const render_model_params &params);
}  // namespace vlk
