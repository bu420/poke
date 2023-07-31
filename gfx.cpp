#include "gfx.hpp"

using namespace poke;

#include <algorithm>

struct line3d {
    vertex start;
    vertex end;
};

enum class line_type {
    any,
    vertical,
    horizontal
};

struct line3d_stepper {
    vertex current;
    vertex increment;

    int steps;
    int i;

    line3d_stepper(line3d line, line_type type) : steps(0), i(0) {
        // Round X and Y to nearest integer (pixel position).
        line.start.position.x() = std::round(line.start.position.x());
        line.start.position.y() = std::round(line.start.position.y());
        line.end.position.x() = std::round(line.end.position.x());
        line.end.position.y() = std::round(line.end.position.y());

        current = line.start;

        vec4f difference(line.end.position - line.start.position);

        // Calculate steps (total number of increments).
        switch (type) {
        case line_type::any:
            steps = static_cast<int>(std::max(std::abs(difference.x()), std::abs(difference.y())));
            break;

        case line_type::vertical:
            steps = static_cast<int>(std::abs(difference.y()));
            break;

        case line_type::horizontal:
            steps = static_cast<int>(std::abs(difference.x()));
        }

        // Calculate how much to increment each step.
        if (steps > 0) {
            increment.position = difference / static_cast<float>(steps);

            assert(line.start.attribute_count == line.end.attribute_count);

            for (int i = 0; i < line.start.attribute_count; i++) {
                assert(line.start.attributes[i].count == line.end.attributes[i].count);

                for (int j = 0; j < line.start.attributes[i].count; j++) {
                    increment.attributes[i].data[j] =
                        (line.end.attributes[i].data[j] - line.start.attributes[i].data[j]) / steps;

                    increment.attributes[i].count = line.start.attributes[i].count;
                }
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
        for (int j = 0; j < current.attribute_count; j++) {
            for (int k = 0; k < current.attributes[j].count; k++) {
                current.attributes[j].data[k] += increment.attributes[j].data[k];
            }
        }

        return true;
    }
};

attribute attribute::lerp(const attribute& other, float amount) const {
    assert(this->count == other.count && "Attribute sizes must match.");

    attribute result;
    for (int i = 0; i < this->count; i++) {
        result.data[i] = std::lerp(this->data[i], other.data[i], amount);
    }
    result.count = this->count;

    return result;
}

vertex vertex::lerp(const vertex& other, float amount) const {
    assert(this->attribute_count == other.attribute_count && "Number of attributes must match.");

    vertex result;

    // Interpolate position.
    for (int i = 0; i < 4; i++) {
        result.position[i] = std::lerp(this->position[i], other.position[i], amount);
    }

    // Interpolate attributes.
    for (int i = 0; i < this->attribute_count; i++) {
        result.attributes[i] = this->attributes[i].lerp(other.attributes[i], amount);
    }
    result.attribute_count = this->attribute_count;

    return result;
}

void poke::render_triangle(
    optional_reference<color_buffer> color_buf,
    optional_reference<depth_buffer> depth_buf,
    std::array<vertex, 3> vertices,
    std::function<byte3(const vertex&)> pixel_shader_callback) {
    assert(color_buf.has_value() || depth_buf.has_value() && "Either a color buffer, depth buffer or both must be present.");

    // W division (homogeneous clip space -> NDC space).
    for (auto& vertex : vertices) {
        auto& pos = vertex.position;

        pos.x() /= pos.w();
        pos.y() /= pos.w();
        pos.z() /= pos.w();
    }

    const vec2i framebuffer_size(
        color_buf.has_value() ?
        vec2i(color_buf.value().get().get_width(), color_buf.value().get().get_height()) :
        vec2i(depth_buf.value().get().get_width(), depth_buf.value().get().get_height()));

    // Viewport transformation. 
    // Scale from [-1, 1] to color buffer size.
    for (auto& vertex : vertices) {
        auto& pos = vertex.position;

        pos.x() = (pos.x() + 1) / 2.f * framebuffer_size.x();
        pos.y() = (pos.y() + 1) / 2.f * framebuffer_size.y();
    }

    vec4f& pos0 = vertices[0].position;
    vec4f& pos1 = vertices[1].position;
    vec4f& pos2 = vertices[2].position;

    // Sort vertices by Y.
    if (pos0.y() > pos1.y()) {
        std::swap(pos0, pos1);
    }
    if (pos0.y() > pos2.y()) {
        std::swap(pos0, pos2);
    }
    if (pos1.y() > pos2.y()) {
        std::swap(pos1, pos2);
    }

    auto render_triangle_from_lines = [&](line3d a, line3d b) {
        // Sort lines based on X.
        if (a.start.position.x() > b.start.position.x()) {
            std::swap(a, b);
        }

        line3d_stepper line_a(a, line_type::vertical);
        line3d_stepper line_b(b, line_type::vertical);

        do {
            assert(line_a.current.position.y() == line_b.current.position.y() && "Big failure.");

            line3d_stepper line_x(line3d{ line_a.current, line_b.current }, line_type::horizontal);

            do {
                int x = static_cast<int>(line_x.current.position.x());
                int y = static_cast<int>(line_x.current.position.y());

                if (depth_buf.has_value()) {
                    float z = line_x.current.position.z();

                    if (z < depth_buf.value().get().at(x, y)) {
                        depth_buf.value().get().at(x, y) = z;
                    }
                    // If pixel is invisible, skip color buffer update.
                    else {
                        continue;
                    }
                }

                if (color_buf.has_value()) {
                    color_buf.value().get().at(x, y) = pixel_shader_callback(line_x.current);
                }
            } while (line_x.step());
        } while (line_a.step() && line_b.step());
    };

    // Check if the top of the triangle is flat.
    if (pos0.y() == pos1.y()) {
        render_triangle_from_lines(line3d{ vertices[0], vertices[2] }, line3d{ vertices[1], vertices[2] });
    }
    // Check if the bottom is flat.
    else if (pos1.y() == pos2.y()) {
        render_triangle_from_lines(line3d{ vertices[0], vertices[1] }, line3d{ vertices[0], vertices[2] });
    }
    // Else split into two smaller triangles.
    else {
        float alpha_split = (pos1.y() - pos0.y()) / (pos2.y() - pos0.y());
        vertex vertex3 = vertices[0].lerp(vertices[2], alpha_split);

        // Top (flat bottom).
        render_triangle_from_lines(line3d{ vertices[0], vertices[1] }, line3d{ vertices[0], vertex3 });

        // Bottom (flat top).
        render_triangle_from_lines(line3d{ vertices[1], vertices[2] }, line3d{ vertex3, vertices[2] });
    }
}