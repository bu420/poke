# Valkyrie

## About
A software rasterizer in C++. 
Visual Studio 2022.
The solution also contains other related projects,
such as the `platform` project which implements platform
specific features like windowing, user input and audio.

## Example
```cpp
vlk::vec4f pos(1.0f, 2.0f, 3.0f, 4.0f);
```

## Custom 3D model format
A custom binary file format (`.vmod`) has been made to be easy to parse. 
Everything is baked into one file.
Not very memory efficient. Meant for small models.

### File content
- Vertices
  - d
- Texture coords
- Normals
- Faces
- Textures
- Materials
