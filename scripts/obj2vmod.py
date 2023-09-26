import sys
import math
import pywavefront
import struct
from PIL import Image

def extract_attributes(scene, identifier):
    result : list[float] = []
    
    for name, material in scene.materials.items():
        # Example format before split: T2F_C3F_N3F_V3F
        format_arr = material.vertex_format.split('_')
        
        size = int(identifier[1])

        offset = 0
        for attribute in format_arr:
            if identifier != attribute:
                offset += int(attribute[1])
            else:
                break

        stride = sum(int(attribute[1]) for attribute in format_arr)

        count = int(len(material.vertices) / stride)

        for i in range(count):
            result.extend(material.vertices[i * stride + offset : i * stride + offset + size])
    
    return result

class Texture:
    def __init__(self, w, h, channels, pixels):
        self.data : list[int] = []
        self.data.append(w)
        self.data.append(h)
        self.data.append(channels)

        # Flatten list of tuples.
        pixels = list(sum(pixels, ()))
        # Pack pixel bytes into ints.
        for i in range(math.ceil(len(pixels) / 4)):
            self.data.append(int.from_bytes([pixels[i * 4 + 0], pixels[i * 4 + 1], pixels[i * 4 + 2], pixels[i * 4 + 3]], byteorder='big', signed=True))

def parse_image(path):
    img = Image.open(path)
    pixels = list(img.getdata())
    w, h = img.size
    channels = len(pixels[0])
    return Texture(w, h, channels, pixels)

def extract_textures(scene):
    result : list[Texture] = []

    for name, material in scene.materials.items():
        texture = material.texture or material.texture_ambient
        if not texture:
            continue
        if texture and not material.has_uvs:
            print('Error: Textured mesh does not have uvs.')
            exit()
        result.append(parse_image(texture._path))
    
    return result

def extract_faces(scene):
    result : list[int] = []

    for mesh in scene.mesh_list:
        for face in mesh.faces:
            result.extend(face)
    
    return result

if len(sys.argv) < 2:
    print('Invalid number of arguments.')
    print('Usage: py obj2vmod.py <obj file name>')
    exit()

'''
Load OBJ model and extract attributes and faces into separate lists.
'''

file_name = sys.argv[1]
scene = pywavefront.Wavefront(file_name, collect_faces=True)

positions : list[float] = extract_attributes(scene, 'V3F')
tex_coords : list[float] = extract_attributes(scene, 'T2F')
normals : list[float] = extract_attributes(scene, 'N3F')
textures : list[Texture] = extract_textures(scene)
faces : list[int] = extract_faces(scene)

''' 
Create VMOD file and write binary data. 
'''

file = open(file_name.split('.')[0] + '.vmod', 'wb')

# 16 byte header reserved at the top for whatever.
file_content = b'\x00' * 16

file_content += struct.pack('>i', int(len(positions) / 3))
file_content += struct.pack('>{}f'.format(len(positions)), *positions)
file_content += struct.pack('>i', int(len(tex_coords) / 2))
file_content += struct.pack('>{}f'.format(len(tex_coords)), *tex_coords)
file_content += struct.pack('>i', int(len(normals) / 3))
file_content += struct.pack('>{}f'.format(len(normals)), *normals)
file_content += struct.pack('>i', len(textures))
for texture in textures:
    print(len(texture.data))
    file_content += struct.pack('>{}i'.format(len(texture.data)), *texture.data)
file_content += struct.pack('>i', int(len(faces) / 3))
file_content += struct.pack('>{}i'.format(len(faces)), *faces)

file.write(bytearray(file_content))

file.close()