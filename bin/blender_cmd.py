import bpy
from mathutils import Matrix, Vector, Euler
import numpy as np
obj = bpy.data.objects['VO']
cam = bpy.data.objects['Camera']
cam.matrix_world = Matrix([[0.77808,-0.218544,0.588923,0.699345],[0.628165,0.270701,-0.729472,-1.025],[0,0.937528,0.347909,0.127262],[0,0,0,1]])
obj.scale = [0.0802576,0.0802576,0.0802576]
bpy.context.object.rotation_mode = 'ZYX'
obj.rotation_euler = Euler([1.6057,-1.13446,0])
obj.location = [-0.171981,-0.10064,-0.422945]
bpy.data.materials['VO'].node_tree.nodes['Glossy BSDF'].inputs[1].default_value = 0.298892
bpy.data.materials['VO'].node_tree.nodes['Mix Shader'].inputs[0].default_value = 0.981813
bpy.data.scenes['Scene'].render.filepath = '/home/lofr/Projects/NeRF/ngp_pl-master/insert/generate/scene0/results/0_blender.exr'
bpy.ops.render.render(write_still=True)