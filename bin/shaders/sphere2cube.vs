#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 textureDir;

uniform mat4 VP;

void main() {
  textureDir = aPos;
  gl_Position = VP * vec4(aPos, 1.0);
}