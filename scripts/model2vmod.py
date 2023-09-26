import pyassimp
import pyassimp.postprocess
import pyassimp.material
import sys

with pyassimp.load(sys.argv[1], processing=pyassimp.postprocess.aiProcess_Triangulate) as scene:
    for mesh in scene.meshes:
        mesh.vertices
        mesh.normals
        mesh.texturecoords

        print(dir(mesh.material))

        for i in mesh.material.properties.items():
            print(i)
            