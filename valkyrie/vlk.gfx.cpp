#include "vlk.gfx.hpp"

#include <algorithm>
#include <fstream>
#include <streambuf>
#include <ranges>

using namespace vlk;

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

    line3d_stepper(line3d line, calc_steps_based_on line_type) {
        assert(line.start.attribute_count == line.end.attribute_count);

        // Round X and Y to nearest integer (pixel position).
        line.start.pos.x() = std::round(line.start.pos.x());
        line.start.pos.y() = std::round(line.start.pos.y());
        line.end.pos.x() = std::round(line.end.pos.x());
        line.end.pos.y() = std::round(line.end.pos.y());

        current = line.start;

        vec4f difference{line.end.pos - line.start.pos};

        // Calculate steps (total number of increments).
        switch (line_type) {
        case calc_steps_based_on::largest_difference:
            steps = static_cast<i32>(std::max(std::abs(difference.x()), std::abs(difference.y())));
            break;

        case calc_steps_based_on::x_difference:
            steps = static_cast<i32>(std::abs(difference.x()));
            break;

        case calc_steps_based_on::y_difference:
            steps = static_cast<i32>(std::abs(difference.y()));
            break;
        }

        if (steps == 0) {
            return;
        }

        // Calculate how much to increment each step.

        increment.pos = difference / static_cast<f32>(steps);
        increment.attribute_count = current.attribute_count;

        for (u8 i{0}; i < line.start.attribute_count; ++i) {
            assert(line.start.attributes[i].size == line.end.attributes[i].size);
            increment.attributes[i].size = line.start.attributes[i].size;

            for (u8 j{0}; j < line.start.attributes[i].size; ++j) {
                increment.attributes[i].data[j] =
                    (line.end.attributes[i].data[j] - 
                     line.start.attributes[i].data[j]) / static_cast<f32>(steps);
            }
        }
    }

    bool step() {
        if (i == steps) {
            return false;
        }

        i++;

        // Increment position.
        current.pos += increment.pos;

        // Increment attributes.
        for (u8 j{0}; j < current.attribute_count; ++j) {
            current.attributes[j] += increment.attributes[j];
        }

        return true;
    }
};

attribute attribute::lerp(const attribute& other, f32 amount) const {
    assert(this->size == other.size);

    attribute result;
    result.size = size;

    for (u8 i{0}; i < size; ++i) {
        result.data[i] = std::lerp(data[i], other.data[i], amount);
    }

    return result;
}

void attribute::operator += (const attribute& a) {
    assert(size == a.size);

    for (u8 i{0}; i < size; ++i) {
        data[i] += a.data[i];
    }
}

vertex vertex::lerp(const vertex& other, f32 amount) const {
    assert(attribute_count == other.attribute_count);

    vertex result;
    result.attribute_count = attribute_count;

    // Interpolate position.
    for (u8 i{0}; i < 4; ++i) {
        result.pos[i] = std::lerp(this->pos[i], other.pos[i], amount);
    }

    // Interpolate attributes.
    for (u8 i{0}; i < this->attribute_count; ++i) {
        result.attributes[i] = this->attributes[i].lerp(other.attributes[i], amount);
    }

    return result;
}

color_rgb vlk::default_color_blend_func(const color_rgb& old_color,
                                   const color_rgba& new_color) {
    // TODO: find fast way to calculate:
    // new_color.rgb * new_color.a + old_color * (1 - new_color.a)

    return {new_color.r, new_color.g, new_color.b};
}

// This function assumes that the entire triangle is visible.
void fill_triangle(const render_triangle_params& params) {
    assert(params.color_buf || params.depth_buf &&
           "Either a color buffer, depth buffer or both must be present.");

    // Make a modifiable copy of the vertices.
    std::array<vertex, 3> vertices = params.vertices;

    // W division (homogeneous clip space -> NDC space).
    for (auto& vertex : vertices) {
        auto& pos = vertex.pos;

        assert(pos.w() != 0);

        pos.x() /= pos.w();
        pos.y() /= pos.w();
        pos.z() /= pos.w();
    }

    const auto framebuffer_size = [&]() -> vec2i {
        if (params.color_buf) {
            return {
                static_cast<i32>(params.color_buf->get().get_width()),
                static_cast<i32>(params.color_buf->get().get_height())
            };
        }
        else {
            return {
                static_cast<i32>(params.depth_buf->get().get_width()),
                static_cast<i32>(params.depth_buf->get().get_height())
            };
        }
    }();

    // Viewport transformation. 
    // Convert [-1, 1] to framebuffer size.
    // Round to nearest pixel pos.
    for (auto& vertex : vertices) {
        auto& pos = vertex.pos;

        pos.x() = std::round((pos.x() + 1.0f) / 2.0f * (static_cast<f32>(framebuffer_size.x()) - 1.0f));
        pos.y() = std::round((pos.y() + 1.0f) / 2.0f * (static_cast<f32>(framebuffer_size.y()) - 1.0f));
    }

    // Position aliases.
    vec4f& p0 = vertices[0].pos;
    vec4f& p1 = vertices[1].pos;
    vec4f& p2 = vertices[2].pos;

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

    auto render_triangle_from_lines = [&](line3d a, line3d b) {
        // Sort lines based on X.
        if (a.start.pos.x() > b.start.pos.x()) {
            std::swap(a, b);
        }

        line3d_stepper line_a(a, line3d_stepper::calc_steps_based_on::y_difference);
        line3d_stepper line_b(b, line3d_stepper::calc_steps_based_on::y_difference);

        do {
            assert(line_a.current.pos.y() == line_b.current.pos.y());

            line3d_stepper line_x(line3d{line_a.current, line_b.current}, line3d_stepper::calc_steps_based_on::x_difference);

            do {
                i32 x = static_cast<i32>(line_x.current.pos.x());
                i32 y = static_cast<i32>(line_x.current.pos.y());

                if (params.depth_buf) {
                    assert(x >= 0 && x < params.depth_buf->get().get_width() &&
                           y >= 0 && y < params.depth_buf->get().get_height());

                    f32& depth_buf_z = params.depth_buf->get().at(x, y);
                    f32 current_z = line_x.current.pos.z();

                    if (current_z < depth_buf_z) {
                        depth_buf_z = current_z;
                    }
                    // If pixel is invisible, skip the next step.
                    else {
                        continue;
                    }
                }

                if (params.color_buf) {
                    assert(x >= 0 && x < params.color_buf->get().get_width() &&
                           y >= 0 && y < params.color_buf->get().get_height());

                    color_rgb& dest = params.color_buf->get().at(x, y);

                    color_rgb old_color = dest;
                    color_rgba new_color = params.pixel_shader(line_x.current, params.user_data);

                    dest = params.color_blend_func(old_color, new_color);
                }
            }
            while (line_x.step());
        }
        while (line_a.step() && line_b.step());
    };

    // Check if the top of the triangle is flat.
    if (p0.y() == p1.y()) {
        render_triangle_from_lines(line3d{vertices[0], vertices[2]}, line3d{vertices[1], vertices[2]});
    }
    // Check if the bottom is flat.
    else if (p1.y() == p2.y()) {
        render_triangle_from_lines(line3d{vertices[0], vertices[1]}, line3d{vertices[0], vertices[2]});
    }
    // Else split into two smaller triangles.
    else {
        float lerp_amount = (p1.y() - p0.y()) / (p2.y() - p0.y());
        vertex vertex3 = vertices[0].lerp(vertices[2], lerp_amount);

        // Top (flat bottom).
        render_triangle_from_lines(line3d{vertices[0], vertices[1]}, line3d{vertices[0], vertex3});

        // Bottom (flat top).
        render_triangle_from_lines(line3d{vertices[1], vertices[2]}, line3d{vertex3, vertices[2]});
    }
}

std::vector<vertex> triangle_clip_component(const std::vector<vertex>& vertices, u8 component_idx) {
    auto clip = [&](const std::vector<vertex>& vertices, f32 sign) {
        std::vector<vertex> result;
        result.reserve(vertices.size());

        for (size_t i{0}; i < vertices.size(); ++i) {
            const vertex& curr_vertex = vertices[i];
            const vertex& prev_vertex = vertices[(i - 1 + vertices.size()) % vertices.size()];

            f32 curr_component = sign * curr_vertex.pos[component_idx];
            f32 prev_component = sign * prev_vertex.pos[component_idx];

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

std::vector<vertex> triangle_clip(const std::array<vertex, 3>& vertices) {
    // Clip X.
    std::vector<vertex> result = triangle_clip_component({vertices.begin(), vertices.end()}, 0);
    if (result.empty()) {
        return result;
    }
    // Clip Y.
    result = triangle_clip_component(result, 1);
    if (result.empty()) {
        return result;
    }
    // Clip Z.
    result = triangle_clip_component(result, 2);
    return result;
}

void vlk::render_triangle(const render_triangle_params& params) {
    auto is_point_visible = [](const vec4f& p) {
        return 
            p.x() >= -p.w() && p.x() <= p.w() && 
            p.y() >= -p.w() && p.y() <= p.w() && 
            p.z() >= -p.w() && p.z() <= p.w();
    };

    // Position aliases.
    const vec4f& p0 = params.vertices[0].pos;
    const vec4f& p1 = params.vertices[1].pos;
    const vec4f& p2 = params.vertices[2].pos;

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
    assert(clipped_vertices.size() >= 3);

    for (size_t i{1}; i < clipped_vertices.size() - 1; ++i) {
        render_triangle_params new_params{params};
        new_params.vertices = {clipped_vertices[0], clipped_vertices[i], clipped_vertices[i + 1]};

        fill_triangle(new_params);
    }
}
