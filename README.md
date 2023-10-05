# Valkyrie

## About
A software rasterizer in C++. 
Visual Studio 2022.

## Example
```cpp
vlk::color_buffer color_buf(600, 400);
vlk::depth_buffer depth_buf(600, 400);

const std::array<vlk::vec4f, 3> positions = <...>;
const std::array<vlk::vec3f, 3> normals = <...>;

const std::array<vlk::vertex, 3> vertices = {
    vlk::vertex(positions[0], vlk::attribute(normals[0])),
    vlk::vertex(positions[1], vlk::attribute(normals[1])),
    vlk::vertex(positions[2], vlk::attribute(normals[2]))
};

auto pixel_shader = 
    [](const vlk::vertex& vertex) -> vlk::byte3 {
        vlk::vec4f pos = vertex.position;

        vlk::vec3f normal(vertex.attributes[0].data.xyz());
        vlk::vec3f color((normal + 1.f) * 127.f);

        return {
            static_cast<unsigned char>(color.x()),
            static_cast<unsigned char>(color.y()),
            static_cast<unsigned char>(color.z())
        };
    }

vlk::render_triangle(color_buf, depth_buf, vertices, pixel_shader);
```
