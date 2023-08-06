#include "gfx.hpp"

#include <algorithm>
#include <fstream>
#include <streambuf>
#include <ranges>

using namespace poke;

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
    // Round X and Y to nearest pixel pos.
    for (auto& vertex : vertices) {
        auto& pos = vertex.position;

        pos.x() = std::round((pos.x() + 1) / 2.f * framebuffer_size.x());
        pos.y() = std::round((pos.y() + 1) / 2.f * framebuffer_size.y());
    }

    // Create aliases for positions.
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

                    color_buf.value().get().at(x, y) = pixel_shader_callback(line_x.current);
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
        float alpha_split = (p1.y() - p0.y()) / (p2.y() - p0.y());
        vertex vertex3 = vertices[0].lerp(vertices[2], alpha_split);

        // Top (flat bottom).
        render_triangle_from_lines(line3d{ vertices[0], vertices[1] }, line3d{ vertices[0], vertex3 });

        // Bottom (flat top).
        render_triangle_from_lines(line3d{ vertices[1], vertices[2] }, line3d{ vertex3, vertices[2] });
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
            const int index_count = tokens.size() - 1;
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