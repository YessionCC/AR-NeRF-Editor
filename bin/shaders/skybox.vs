#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 textureDir;

uniform mat4 VrotP;
uniform mat4 MVP;

void main() {
  textureDir = aPos;
  gl_Position = (VrotP * vec4(aPos, 1.0)).xyww; // always keep its depth = 1.0
}