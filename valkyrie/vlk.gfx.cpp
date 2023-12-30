#include "vlk.gfx.hpp"

#include <algorithm>
#include <fstream>
#include <streambuf>
#include <ranges>

using namespace vlk;

attrib attrib::lerp(const attrib &other, f32 amount) const {
    VLK_ASSERT_FAST(size == other.size, "Attribute sizes must match.");

    vec4f data{};

    for (u32 i = 0; i < size; ++i) {
        data[i] = std::lerp(data[i], other[i], amount);
    }

    return attrib{data, size};
}

void attrib::operator+=(const attrib &a) {
    VLK_ASSERT_FAST(size == a.size, "Attribute sizes must match.");

    for (u32 i = 0; i < size; ++i) {
        data[i] += a[i];
    }
}

vertex vertex::lerp(const vertex &other, f32 amount) const {
    VLK_ASSERT_FAST(count == other.count, "Attribute counts must match.");

    vertex result{pos.lerp(other.pos, amount), count};

    for (u32 i = 0; i < count; ++i) {
        result[i] = m_attribs[i].lerp(other[i], amount);
    }

    return result;
}

line_stepper::line_stepper(line line, calc_steps_based_on line_type) : m_i{0} {
    VLK_ASSERT_FAST(line.start.count == line.end.count, "Attribute counts must match.");

    line.start.pos.x() = std::round(line.start.pos.x());
    line.start.pos.y() = std::round(line.start.pos.y());
    line.end.pos.x()   = std::round(line.end.pos.x());
    line.end.pos.y()   = std::round(line.end.pos.y());

    m_current = line.start;

    const vec4f diff = line.end.pos - line.start.pos;

    // Calculate steps i.e. number of increments.
    switch (line_type) {
        case calc_steps_based_on::largest_diff:
            m_steps = static_cast<i32>(std::max(std::abs(diff.x()), std::abs(diff.y())));
            break;

        case calc_steps_based_on::x_diff: m_steps = static_cast<i32>(std::abs(diff.x())); break;

        case calc_steps_based_on::y_diff: m_steps = static_cast<i32>(std::abs(diff.y())); break;
    }

    if (m_steps == 0) {
        return;
    }

    // Calculate how much to increment at each step.

    m_increment.pos   = diff / static_cast<f32>(m_steps);
    m_increment.count = m_current.count;

    for (u32 i = 0; i < line.start.count; ++i) {
        VLK_ASSERT_FAST(line.start[i].size == line.end[i].size, "attrib counts must match.");

        m_increment[i].size = m_current[i].size;

        for (u32 j = 0; j < m_current[i].size; ++j) {
            m_increment[i][j] = (line.end[i][j] - line.start[i][j]) / static_cast<f32>(m_steps);
        }
    }
}

bool line_stepper::step() {
    if (m_i == m_steps) {
        return false;
    }

    m_i++;

    m_current.pos += m_increment.pos;

    for (u32 i = 0; i < m_current.count; ++i) {
        m_current[i] += m_increment[i];
    }

    return true;
}

color_rgba vlk::default_color_blend(const color_rgba &old_color, const color_rgba &new_color) {
    // TODO: find fast way to calculate:
    // new_color.rgb * new_color.a + old_color * (1 - new_color.a)

    return new_color;
}

enum class component {
    x = 0,
    y = 1,
    z = 2
};

static std::vector<vertex> triangle_clip_component(const std::vector<vertex> &vertices, component component) {
    auto clip = [&](const std::vector<vertex> &vertices, f32 sign) {
        std::vector<vertex> result;
        result.reserve(vertices.size());

        for (size_t i = 0; i < vertices.size(); ++i) {
            const vertex &curr_vertex = vertices[i];
            const vertex &prev_vertex = vertices[(i - 1 + vertices.size()) % vertices.size()];

            f32 curr_component = sign * curr_vertex.pos[static_cast<i32>(component)];
            f32 prev_component = sign * prev_vertex.pos[static_cast<i32>(component)];

            bool curr_is_inside = curr_component <= curr_vertex.pos.w();
            bool prev_is_inside = prev_component <= prev_vertex.pos.w();

            if (curr_is_inside != prev_is_inside) {
                float lerp_amount =
                    (prev_vertex.pos.w() - prev_component) /
                    ((prev_vertex.pos.w() - prev_component) - (curr_vertex.pos.w() - curr_component));

                result.emplace_back(prev_vertex.lerp(curr_vertex, lerp_amount));
            }

            if (curr_is_inside) {
                result.emplace_back(curr_vertex);
            }
        }

        return result;
    };

    std::vector<vertex> result = clip(vertices, 1.0f);
    if (result.empty()) {
        return result;
    }
    return clip(result, -1.0f);
}

static std::vector<vertex> triangle_clip(const std::array<vertex, 3> &vertices) {
    std::vector<vertex> result = triangle_clip_component({vertices.begin(), vertices.end()}, component::x);

    if (result.empty()) {
        return result;
    }

    result = triangle_clip_component(result, component::y);
    if (result.empty()) {
        return result;
    }

    result = triangle_clip_component(result, component::z);
    return result;
}

static void raster_triangle_scanline(const render_triangle_params &params, line a, line b) {
    if (a.start.pos.x() > b.start.pos.x()) {
        std::swap(a, b);
    }

    line_stepper line_a{a, line_stepper::calc_steps_based_on::y_diff};
    line_stepper line_b{b, line_stepper::calc_steps_based_on::y_diff};

    do {
        VLK_ASSERT_FAST(line_a.vertex().pos.y() == line_b.vertex().pos.y(),
                        "Y coordinate must the be same for both lines.");

        line_stepper line_x{
            line{line_a.vertex(), line_b.vertex()},
            line_stepper::calc_steps_based_on::x_diff
        };

        do {
            i32 x = static_cast<i32>(line_x.vertex().pos.x());
            i32 y = static_cast<i32>(line_x.vertex().pos.y());

            if (params.depth_buf) {
                VLK_ASSERT_FAST(x >= 0 && x < params.depth_buf->get().width() && y >= 0 &&
                                    y < params.depth_buf->get().height(),
                                "Outside bounds.");

                f32 &depth_buf_z = params.depth_buf->get().at(x, y);
                f32 current_z    = line_x.vertex().pos.z();

                if (current_z < depth_buf_z) {
                    depth_buf_z = current_z;
                }
                // If the pixel is invisible, do not run the pixel shader.
                else {
                    continue;
                }
            }

            if (params.color_buf) {
                VLK_ASSERT_FAST(x >= 0 && x < params.color_buf->get().width() && y >= 0 &&
                                    y < params.color_buf->get().height(),
                                "Outside bounds.");

                color_rgba &dest = params.color_buf->get().at(x, y);

                color_rgba old_color = dest;
                color_rgba new_color = params.pixel_shader(line_x.vertex());

                dest = params.color_blend(old_color, new_color);
            }
        } while (line_x.step());
    } while (line_a.step() && line_b.step());
}

// NOTE: The entire triangle MUST be visible.
static void fill_triangle(const render_triangle_params &params) {
    // Make a modifiable copy of the vertices.
    std::array<vertex, 3> vertices = params.vertices;

    // W division (homogeneous clip space -> NDC space).
    for (auto &vertex : vertices) {
        auto &pos = vertex.pos;

        assert(pos.w() != 0);

        pos.x() /= pos.w();
        pos.y() /= pos.w();
        pos.z() /= pos.w();
    }

    const auto framebuffer_size = [&]() -> vec2i {
        if (params.color_buf) {
            return {static_cast<i32>(params.color_buf->get().width()),
                    static_cast<i32>(params.color_buf->get().height())};
        } else {
            return {static_cast<i32>(params.depth_buf->get().width()),
                    static_cast<i32>(params.depth_buf->get().height())};
        }
    }();

    // Viewport transformation.
    // Convert [-1, 1] to framebuffer size.
    // Round to nearest pixel pos.
    for (auto &vertex : vertices) {
        auto &pos = vertex.pos;

        pos.x() = std::round((pos.x() + 1.0f) / 2.0f * (static_cast<f32>(framebuffer_size.x()) - 1.0f));
        pos.y() = std::round((pos.y() + 1.0f) / 2.0f * (static_cast<f32>(framebuffer_size.y()) - 1.0f));
    }

    // Position aliases.
    vec4f &p0 = vertices[0].pos;
    vec4f &p1 = vertices[1].pos;
    vec4f &p2 = vertices[2].pos;

    // Sort vertices by Y.
    if (p0.y() > p1.y()) {
        std::swap(vertices[0], vertices[1]);
    }
    if (p0.y() > p2.y()) {
        std::swap(vertices[0], vertices[2]);
    }
    if (p1.y() > p2.y()) {
        std::swap(vertices[1], vertices[2]);
    }

    // Check if the top of the triangle is flat.
    if (p0.y() == p1.y()) {
        raster_triangle_scanline(params, line{vertices[0], vertices[2]}, line{vertices[1], vertices[2]});
    }
    // Check if the bottom is flat.
    else if (p1.y() == p2.y()) {
        raster_triangle_scanline(params, line{vertices[0], vertices[1]}, line{vertices[0], vertices[2]});
    }
    // Else split into two smaller triangles.
    else {
        float lerp_amount = (p1.y() - p0.y()) / (p2.y() - p0.y());
        vertex vertex_3   = vertices[0].lerp(vertices[2], lerp_amount);

        // Top (flat bottom).
        raster_triangle_scanline(params, line{vertices[0], vertices[1]}, line{vertices[0], vertex_3});

        // Bottom (flat top).
        raster_triangle_scanline(params, line{vertices[1], vertices[2]}, line{vertex_3, vertices[2]});
    }
}

void vlk::render_triangle(const render_triangle_params &params) {
    VLK_ASSERT_FAST(params.color_buf || params.depth_buf,
                    "Either a color buffer, depth buffer or both must be present.");

    auto is_point_visible = [](const vec4f &p) {
        return p.x() >= -p.w() && p.x() <= p.w() && p.y() >= -p.w() && p.y() <= p.w() && p.z() >= -p.w() &&
               p.z() <= p.w();
    };

    // Position aliases.
    const vec4f &p0 = params.vertices[0].pos;
    const vec4f &p1 = params.vertices[1].pos;
    const vec4f &p2 = params.vertices[2].pos;

    bool is_p0_visible = is_point_visible(p0);
    bool is_p1_visible = is_point_visible(p1);
    bool is_p2_visible = is_point_visible(p2);

    // If all points are visible, draw triangle.
    if (is_p0_visible && is_p1_visible && is_p2_visible) {
        fill_triangle(params);
        return;
    }
    // If no vertices are visible, discard triangle.
    if (!is_p0_visible && !is_p1_visible && !is_p2_visible) {
        return;
    }

    // Else clip triangle.

    std::vector<vertex> clipped_vertices = triangle_clip(params.vertices);
    VLK_ASSERT_FAST(clipped_vertices.size() >= 3, "Can't render a polygon with less than 3 vertices.");

    for (size_t i = 1; i < clipped_vertices.size() - 1; ++i) {
        render_triangle_params new_params{params};
        new_params.vertices = {clipped_vertices[0], clipped_vertices[i], clipped_vertices[i + 1]};

        fill_triangle(new_params);
    }
}
image::image(size_t width, size_t height, size_t channels)
    : m_width{width}, m_height{height}, m_channels{channels} {
    m_data.resize(width * height * channels);
}

std::vector<u8>::iterator image::at(size_t x, size_t y) {
    VLK_ASSERT_FAST(x >= 0 && x < m_width && y >= 0 && y < m_height, "Outside bounds.");
    return m_data.begin() + (y * m_width + x) * m_channels;
}

std::vector<u8>::const_iterator image::at(size_t x, size_t y) const {
    VLK_ASSERT_FAST(x >= 0 && x < m_width && y >= 0 && y < m_height, "Outside bounds.");
    return m_data.begin() + (y * m_width + x) * m_channels;
}

std::vector<u8>::iterator image::sample(f32 x, f32 y) {
    return at(static_cast<size_t>(std::round(x * static_cast<f32>(m_width - 1))),
              static_cast<size_t>(std::round(y * static_cast<f32>(m_height - 1))));
}

std::vector<u8>::const_iterator image::sample(f32 x, f32 y) const {
    return at(static_cast<size_t>(std::round(x * static_cast<f32>(m_width - 1))),
              static_cast<size_t>(std::round(y * static_cast<f32>(m_height - 1))));
}

image image::flip_vertically(const image &image) {
    vlk::image result{image.width(), image.height(), image.channels()};

    for (size_t x = 0; x < image.width(); ++x) {
        for (size_t y = 0; y < image.height() / 2; ++y) {
            for (size_t channel = 0; channel < image.channels(); ++channel) {
                *(result.at(x, y) + channel) = *(image.at(x, image.height() - y - 1) + channel);

                *(result.at(x, image.height() - y - 1) + channel) = *(image.at(x, y) + channel);
            }
        }
    }

    return result;
}

color_rgba vlk::default_model_pixel_shader(const vertex &vertex, const model &model, size_t material_index) {
    const vec2f tex_coord = vertex[0].data.xy();
    const vec3f normal    = vertex[1].data.xyz();

    color_rgba result{255, 0, 255, 255};

    if (material_index == model::null_index) {
        return result;
    }

    const model::material &material = model.materials[material_index];

    if (material.albedo != model::null_index) {
        auto pixel = model.images[material.albedo].sample(tex_coord.x(), tex_coord.y());
        result.r   = *pixel;
        result.g   = *(pixel + 1);
        result.b   = *(pixel + 2);
        result.a   = *(pixel + 3);
    }

    return result;
}

void vlk::render_model(const render_model_params &params) {
    for (auto &mesh : params.model.meshes) {
        for (auto &face : mesh.faces) {
            std::array<vertex, 3> vertices{vertex{{params.model.positions[face.positions[0]], 1.0f}},
                                           vertex{{params.model.positions[face.positions[1]], 1.0f}},
                                           vertex{{params.model.positions[face.positions[2]], 1.0f}}};

            for (auto &vertex : vertices) {
                vertex.pos *= params.mvp_matrix;
            }

            if (mesh.has_tex_coords) {
                for (u8 i = 0; i < 3; ++i) {
                    attrib &attrib         = vertices[i][vertices[i].count++];
                    const vec2f &tex_coord = params.model.tex_coords[face.tex_coords[i]];

                    attrib.data.x() = tex_coord.x();
                    attrib.data.y() = tex_coord.y();
                    attrib.size     = 2;
                }
            }
            if (mesh.has_normals) {
                for (u8 i = 0; i < 3; ++i) {
                    attrib &attrib      = vertices[i][vertices[i].count++];
                    const vec3f &normal = params.model.normals[face.normals[i]] * params.normal_matrix;

                    attrib.data.x() = normal.x();
                    attrib.data.y() = normal.y();
                    attrib.data.z() = normal.z();
                    attrib.size     = 3;
                }
            }

            render_triangle({.vertices     = vertices,
                             .color_buf    = params.color_buf,
                             .depth_buf    = params.depth_buf,
                             .pixel_shader = [&](const vertex &vertex) {
                                 return params.pixel_shader(vertex, params.model, mesh.material_index);
                             }});
        }
    }
}