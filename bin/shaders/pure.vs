#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 vNormal;

uniform mat4 MVP;
uniform mat3 Normal2Cam;

void main() {
  vNormal = Normal2Cam*aNormal;
  gl_Position = MVP * vec4(aPos, 1.0);
}