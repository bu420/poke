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
The framework uses a custom binary file format (`.vmod`) where 
everything is baked into one file.
Not very memory efficient. Meant for small models.
Big-endian.

### File content
| Bytes | Type        | Description                                              |
|-------|-------------|----------------------------------------------------------|
| 4     | int         | Number of positions                                      |
| 12    | 3 x float   | 1st position [x,y,z]                                     |
| 12    | 3 x float   | 2nd position                                             |
| 12    | 3 x float   | 3rd position, etc...                                     |
| 4     | int         | Number of texture coordinates                            |
| 8     | 2 x float   | 1st texture coordinate [u,v]                             |
| 8     | 2 x float   | 2nd texture coordinate                                   |
| 8     | 2 x float   | 3rd texture coordinate, etc...                           |
| 4     | int         | Number of normals                                        |
| 12    | 3 x float   | 1st normal [x,y,z]                                       |
| 12    | 3 x float   | 2nd normal                                               |
| 12    | 3 x float   | 3rd normal, etc...                                       |
| 4     | int         | Number of faces                                          |
| 36    | 3 x 3 x int | 1st face, 3 x [pos index, tex coord index, normal index] |
| 36    | 3 x 3 x int | 2nd face                                                 |
| 36    | 3 x 3 x int | 3rd face, etc...                                         |
|       |             |                                                          |
|       |             |                                                          |
|       |             |                                                          |
