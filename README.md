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
Valkyrie uses a custom 3d model binary format (`.vmod`) where 
positions, texture coordinates, normals and textures are baked into a single file.
Not very memory efficient. Meant for small models.
Big-endian.
Integers and floats are both 4 bytes in size.

| File content                                                                                                                                                                                                                                                                                     |
|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Header. 16 bytes. Unused.                                                                                                                                                                                                                                                                        |
| Position count (int).                                                                                                                                                                                                                                                                            |
| Positions. Each element consists of 3 floats [x, y, z].                                                                                                                                                                                                                                          |
| Texture coordinate count (int).                                                                                                                                                                                                                                                                  |
| Texture coordinates. Each element consists of 2 floats [u, v].                                                                                                                                                                                                                                   |
| Normal count (int).                                                                                                                                                                                                                                                                              |
| Normals. Each element consists of 3 floats [x, y, z].                                                                                                                                                                                                                                            |
| Texture count (int).                                                                                                                                                                                                                                                                             |
| Textures. Each element consists of 2 ints (width and height), 1 int specifying number of channels and then the pixels. The size of each pixel is determined by the number of channels. Each channel is 1 byte. If number of channels is set to 4, then each pixel would be 4 bytes [r, g, b, a]. |
| Face count (int).                                                                                                                                                                                                                                                                                |
| Faces. Each element is 40 bytes. 3 x [position index, texture coord index, normal index] + texture index.                                                                                                                                                                                        |