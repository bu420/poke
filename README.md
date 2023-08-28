# Valkyrie

## About
A software rasterizer in C++. 
Visual Studio 2022.
The solution also contains other related projects,
such as the `platform` project which implements platform
specific features like windowing, user input and audio.

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

## Custom 3D model format
The framework uses a custom binary file format (`.vmod`) where 
everything is baked into one file.
Not very memory efficient. Meant for small models.
Big-endian.

### File content
| Bytes  | Type          | Description                                                                                         |
|--------|---------------|-----------------------------------------------------------------------------------------------------|
| 16     |               | Header. Unused.                                                                                     |
| 4      | int           | Position count (P).                                                                                 |
| 4      | int           | Tex coord count (T).                                                                                |
| 4      | int           | Normal count (N).                                                                                   |
| 4      | int           | Face count (F).                                                                                     |
| P x 12 | P x 3 x float | Position data [X,Y,Z].                                                                              |
| T x 8  | T x 2 x float | Tex coord data [U,V].                                                                               |
| N x 12 | N x 3 x float | Normal data [X,Y,Z].                                                                                |
| F x 12 | F x 3 x int   | Face data. Each face consists of a position index, tex coord index and normal index, in that order. |