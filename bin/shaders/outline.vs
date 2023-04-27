#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;


uniform mat4 MVP;

void main() { 
  vec4 pos = MVP * vec4(aPos, 1.0);
  gl_Position = vec4(pos.xy, 0, pos.w);
}