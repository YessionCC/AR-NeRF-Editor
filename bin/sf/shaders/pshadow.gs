#version 330 core
layout (triangles) in; // input is a triangle with three vetices
// output is 6 triangles that projected to 6 faces and will be drawn on 6
// textures of cubemap respectively
layout (triangle_strip, max_vertices=18) out;

// VP to project to 6 faces
uniform mat4 ShadowMatrices[6];

// FragPos from GS (output per emitvertex)
out vec4 FragPos; 

void main() {
  for(int face = 0; face < 6; ++face) {
    gl_Layer = face; // built-in variable that specifies to which face we render.
    for(int i = 0; i < 3; ++i) {// for each triangle's vertices
      // gl_in is the input triangles, its vertex can be visited by index
      FragPos = gl_in[i].gl_Position;
      // gl_Position will be divide by w auto before sending to frag shader
      gl_Position = ShadowMatrices[face] * FragPos;

      // each time we complete processing, EmitVertex should be called
      // three vertices emitted will construct a triangle and the fourth will
      // construct the second triangle with the second and the third vertex
      EmitVertex();
    }    
    // each time we complete a primtive, EndPrimitive should be called,
    // so next vertices emitted will not construct triangles with vertices before.
    EndPrimitive();
  }
}