#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in vec3 aSH9_1;
layout (location = 6) in vec3 aSH9_2;
layout (location = 7) in vec3 aSH9_3;

out vec3 SH9_1;
out vec3 SH9_2;
out vec3 SH9_3;
uniform mat4 MVP;

void main() {
  SH9_1 = aSH9_1;
  SH9_2 = aSH9_2;
  SH9_3 = aSH9_3;
  gl_Position = MVP * vec4(aPos, 1.0);
}