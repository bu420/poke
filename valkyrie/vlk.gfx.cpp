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

    i32 steps;
    i32 i;

    line3d_stepper(line3d line, calc_steps_based_on line_type) : i{0} {
        assert(line.start.attribute_count == line.end.attribute_count);

        // Round X and Y to nearest integer (pixel position).
        line.start.position.x() = std::round(line.start.position.x());
        line.start.position.y() = std::round(line.start.position.y());
        line.end.position.x() = std::round(line.end.position.x());
        line.end.position.y() = std::round(line.end.position.y());

        current = line.start;

        vec4f difference{line.end.position - line.start.position};

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

        increment.position = difference / static_cast<f32>(steps);
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
        current.position += increment.position;

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
        result.position[i] = std::lerp(this->position[i], other.position[i], amount);
    }

    // Interpolate attributes.
    for (u8 i{0}; i < this->attribute_count; ++i) {
        result.attributes[i] = this->attributes[i].lerp(other.attributes[i], amount);
    }

    return result;
}

// This function assumes that the entire triangle is visible.
void fill_triangle(std::array<vertex, 3> vertices, 
                   optional_ref<color_buffer> color_buf,
                   optional_ref<depth_buffer> depth_buf,
                   pixel_shader_callback pixel_shader) {
    assert(color_buf.has_value() || depth_buf.has_value() &&
           "Either a color buffer, depth buffer or both must be present.");

    // W division (homogeneous clip space -> NDC space).
    for (auto& vertex : vertices) {
        auto& pos = vertex.position;

        assert(pos.w() != 0);

        pos.x() /= pos.w();
        pos.y() /= pos.w();
        pos.z() /= pos.w();
    }

    const vec2i framebuffer_size(color_buf.has_value() ?
                                 vec2i(static_cast<i32>(color_buf.value().get().get_width()), static_cast<i32>(color_buf.value().get().get_height())) :
                                 vec2i(static_cast<i32>(depth_buf.value().get().get_width()), static_cast<i32>(depth_buf.value().get().get_height())));

    // Viewport transformation. 
    // Convert [-1, 1] to framebuffer size.
    // Round to nearest pixel pos.
    for (auto& vertex : vertices) {
        auto& pos = vertex.position;

        pos.x() = std::round((pos.x() + 1.0f) / 2.0f * (static_cast<f32>(framebuffer_size.x()) - 1.0f));
        pos.y() = std::round((pos.y() + 1.0f) / 2.0f * (static_cast<f32>(framebuffer_size.y()) - 1.0f));
    }

    // Position aliases.
    vec4f& p0 = vertices[0].position;
    vec4f& p1 = vertices[1].position;
    vec4f& p2 = vertices[2].position;

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
        if (a.start.position.x() > b.start.position.x()) {
            std::swap(a, b);
        }

        line3d_stepper line_a(a, line3d_stepper::calc_steps_based_on::y_difference);
        line3d_stepper line_b(b, line3d_stepper::calc_steps_based_on::y_difference);

        do {
            assert(line_a.current.position.y() == line_b.current.position.y() && "Big failure.");

            line3d_stepper line_x(line3d{line_a.current, line_b.current}, line3d_stepper::calc_steps_based_on::x_difference);

            do {
                i32 x = static_cast<i32>(line_x.current.position.x());
                i32 y = static_cast<i32>(line_x.current.position.y());

                if (depth_buf.has_value()) {
                    assert(x >= 0 && x < depth_buf.value().get().get_width() &&
                           y >= 0 && y < depth_buf.value().get().get_height());

                    f32 z = line_x.current.position.z();

                    if (z < depth_buf.value().get().at(x, y)) {
                        depth_buf.value().get().at(x, y) = z;
                    }
                    // If pixel is invisible, skip color buffer update.
                    else {
                        continue;
                    }
                }

                if (color_buf.has_value()) {
                    assert(x >= 0 && x < color_buf.value().get().get_width() &&
                           y >= 0 && y < color_buf.value().get().get_height());

                    std::optional<byte3> color = pixel_shader(line_x.current);

                    if (color.has_value()) {
                        color_buf.value().get().at(x, y) = color.value();
                    }
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

std::vector<vertex> triangle_clip_component(const std::vector<vertex>& vertices, int component_idx) {
    auto clip = [&](const std::vector<vertex>& vertices, float sign) {
        std::vector<vertex> result;
        result.reserve(vertices.size());

        for (size_t i{0}; i < vertices.size(); ++i) {
            const vertex& curr_vertex = vertices[i];
            const vertex& prev_vertex = vertices[(i - 1 + vertices.size()) % vertices.size()];

            f32 curr_component = sign * curr_vertex.position[component_idx];
            f32 prev_component = sign * prev_vertex.position[component_idx];

            bool curr_is_inside = curr_component <= curr_vertex.position.w();
            bool prev_is_inside = prev_component <= prev_vertex.position.w();

            if (curr_is_inside != prev_is_inside) {
                float lerp_amount =
                    (prev_vertex.position.w() - prev_component) /
                    ((prev_vertex.position.w() - prev_component) - (curr_vertex.position.w() - curr_component));

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

void vlk::render_triangle(std::array<vertex, 3> vertices, 
                          optional_ref<color_buffer> color_buf,
                          optional_ref<depth_buffer> depth_buf,
                          pixel_shader_callback pixel_shader) {
    auto is_point_visible = [](const vec4f& p) {
        return p.x() >= -p.w() && p.x() <= p.w() && p.y() >= -p.w() && p.y() <= p.w() && p.z() >= -p.w() && p.z() <= p.w();
    };

    // Position aliases.
    vec4f& p0 = vertices[0].position;
    vec4f& p1 = vertices[1].position;
    vec4f& p2 = vertices[2].position;

    bool is_p0_visible = is_point_visible(p0);
    bool is_p1_visible = is_point_visible(p1);
    bool is_p2_visible = is_point_visible(p2);

    // If all points are visible, draw triangle.
    if (is_p0_visible && is_p1_visible && is_p2_visible) {
        fill_triangle(vertices, color_buf, depth_buf, pixel_shader);
        return;
    }
    // If no vertices are visible, discard triangle.
    if (!is_p0_visible && !is_p1_visible && !is_p2_visible) {
        return;
    }

    // Else clip triangle.

    std::vector<vertex> clipped_vertices = triangle_clip(vertices);
    assert(clipped_vertices.size() >= 3);

    for (size_t i{1}; i < clipped_vertices.size() - 1; ++i) {
        fill_triangle({clipped_vertices[0], clipped_vertices[i], clipped_vertices[i + 1]}, 
                      color_buf, 
                      depth_buf, 
                      pixel_shader);
    }
}
