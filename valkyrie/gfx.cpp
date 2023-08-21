#include "gfx.hpp"

#include <algorithm>
#include <fstream>
#include <streambuf>
#include <ranges>

using namespace vlk;

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
            increment.attribute_count = current.attribute_count;

            assert(line.start.attribute_count == line.end.attribute_count);

            for (int i = 0; i < line.start.attribute_count; i++) {
                assert(line.start.attributes[i].count == line.end.attributes[i].count);

                for (int j = 0; j < line.start.attributes[i].count; j++) {
                    increment.attributes[i].data[j] =
                        (line.end.attributes[i].data[j] - line.start.attributes[i].data[j]) / steps;
                }
                increment.attributes[i].count = line.start.attributes[i].count;
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
            current.attributes[j] += increment.attributes[j];
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

// This function assumes that the entire triangle is visible.
void fill_triangle(
    optional_reference<color_buffer> color_buf,
    optional_reference<depth_buffer> depth_buf,
    std::array<vertex, 3> vertices,
    pixel_shader_callback pixel_shader) {
    assert(color_buf.has_value() || depth_buf.has_value() && "Either a color buffer, depth buffer or both must be present.");

    // W division (homogeneous clip space -> NDC space).
    for (auto& vertex : vertices) {
        auto& pos = vertex.position;

        assert(pos.w() != 0);

        pos.x() /= pos.w();
        pos.y() /= pos.w();
        pos.z() /= pos.w();
    }

    const vec2i framebuffer_size(
        color_buf.has_value() ?
        vec2i(color_buf.value().get().get_width(), color_buf.value().get().get_height()) :
        vec2i(depth_buf.value().get().get_width(), depth_buf.value().get().get_height()));

    // Viewport transformation. 
    // Convert [-1, 1] to framebuffer size.
    // Round to nearest pixel pos.
    for (auto& vertex : vertices) {
        auto& pos = vertex.position;

        pos.x() = std::round((pos.x() + 1) / 2.f * (framebuffer_size.x() - 1));
        pos.y() = std::round((pos.y() + 1) / 2.f * (framebuffer_size.y() - 1));
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

        line3d_stepper line_a(a, line_type::vertical);
        line3d_stepper line_b(b, line_type::vertical);

        do {
            assert(line_a.current.position.y() == line_b.current.position.y() && "Big failure.");

            line3d_stepper line_x(line3d{ line_a.current, line_b.current }, line_type::horizontal);

            do {
                int x = static_cast<int>(line_x.current.position.x());
                int y = static_cast<int>(line_x.current.position.y());

                if (depth_buf.has_value()) {
                    if (!(x >= 0 && x < depth_buf.value().get().get_width() &&
                        y >= 0 && y < depth_buf.value().get().get_height()))
                        continue;
                    assert(x >= 0 && x < depth_buf.value().get().get_width() &&
                        y >= 0 && y < depth_buf.value().get().get_height());

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
                    assert(x >= 0 && x < color_buf.value().get().get_width() &&
                        y >= 0 && y < color_buf.value().get().get_height());

                    std::optional<byte3> color = pixel_shader(line_x.current);

                    if (color.has_value()) {
                        color_buf.value().get().at(x, y) = color.value();
                    }
                }
            } while (line_x.step());
        } while (line_a.step() && line_b.step());
    };

    // Check if the top of the triangle is flat.
    if (p0.y() == p1.y()) {
        render_triangle_from_lines(line3d{ vertices[0], vertices[2] }, line3d{ vertices[1], vertices[2] });
    }
    // Check if the bottom is flat.
    else if (p1.y() == p2.y()) {
        render_triangle_from_lines(line3d{ vertices[0], vertices[1] }, line3d{ vertices[0], vertices[2] });
    }
    // Else split into two smaller triangles.
    else {
        float lerp_amount = (p1.y() - p0.y()) / (p2.y() - p0.y());
        vertex vertex3 = vertices[0].lerp(vertices[2], lerp_amount);

        // Top (flat bottom).
        render_triangle_from_lines(line3d{ vertices[0], vertices[1] }, line3d{ vertices[0], vertex3 });

        // Bottom (flat top).
        render_triangle_from_lines(line3d{ vertices[1], vertices[2] }, line3d{ vertex3, vertices[2] });
    }
}

std::vector<vertex> triangle_clip_component(const std::vector<vertex>& vertices, int component_idx) {    
    auto clip = [&](const std::vector<vertex>& vertices, float sign) {
        std::vector<vertex> result;
        result.reserve(vertices.size());

        for (int i = 0; i < vertices.size(); i++) {
            const vertex& curr_vertex = vertices[i];
            const vertex& prev_vertex = vertices[(i - 1 + vertices.size()) % vertices.size()];

            float curr_component = sign * curr_vertex.position[component_idx];
            float prev_component = sign * prev_vertex.position[component_idx];

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
    std::vector<vertex> result = triangle_clip_component({ vertices.begin(), vertices.end() }, 0);
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

void vlk::render_triangle(
    optional_reference<color_buffer> color_buf,
    optional_reference<depth_buffer> depth_buf,
    std::array<vertex, 3> vertices,
    pixel_shader_callback pixel_shader) {
    auto is_point_visible = [](const vec4f& p) {
        return p.x() >= -p.w() && p.x() <= p.w() && p.y() >= -p.w() && p.y() <= p.w() && p.z() >= -p.w() && p.z() <= p.w();
    };

    vec4f& p0 = vertices[0].position;
    vec4f& p1 = vertices[1].position;
    vec4f& p2 = vertices[2].position;

    bool is_p0_visible = is_point_visible(p0);
    bool is_p1_visible = is_point_visible(p1);
    bool is_p2_visible = is_point_visible(p2);

    // If all points are visible, draw triangle.
    if (is_p0_visible && is_p1_visible && is_p2_visible) {
        fill_triangle(color_buf, depth_buf, vertices, pixel_shader);
        return;
    }
    // If all vertices are outside view, discard triangle.
    if (!is_p0_visible && !is_p1_visible && !is_p2_visible) {
        return;
    }

    // Else clip triangle.

    std::vector<vertex> clipped_vertices = triangle_clip(vertices);
    assert(clipped_vertices.size() >= 3);
   
    for (int i = 1; i < clipped_vertices.size() - 1; i++) {
        fill_triangle(color_buf, depth_buf, { clipped_vertices[0], clipped_vertices[i], clipped_vertices[i + 1] }, pixel_shader);
    }
}

bool mesh::load_obj(const std::string& path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        return false;
    }

    std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    for (const auto& line : string_split(file_content, "\n")) {
        const auto& tokens = string_split(line, " ");

        if (tokens.size() == 0) {
            continue;
        }

        if (tokens[0] == "v") {
            positions.push_back(vec3f(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3])));
        }
        else if (tokens[0] == "vt") {
            tex_coords.push_back(vec2f(std::stof(tokens[1]), std::stof(tokens[2])));
        }
        else if (tokens[0] == "vn") {
            normals.push_back(vec3f(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3])));
        }
        else if (tokens[0] == "f") {
            const int index_count = static_cast<int>(tokens.size()) - 1;
            std::vector<mesh::face::index> indices(index_count);

            for (int i = 0; i < index_count; i++) {
                const auto& face_tokens = string_split(tokens[1 + i], "/");
                
                // Remove 1 from each index to convert them from 1-based to 0-based.
                indices[i].pos = std::stoi(face_tokens[0]) - 1;
                indices[i].tex_coord = std::stoi(face_tokens[1]) - 1;
                indices[i].normal = std::stoi(face_tokens[2]) - 1;
            }

            const int face_count = index_count - 2;

            for (int i = 0; i < face_count; i++) {
                mesh::face face;
                face.indices[0] = indices[(i * 2 + 0) % index_count];
                face.indices[1] = indices[(i * 2 + 1) % index_count];
                face.indices[2] = indices[(i * 2 + 2) % index_count];

                faces.push_back(face);
            }
        }
    }

    file.close();
    return true;
}