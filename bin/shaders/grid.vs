#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 worldCoordNear;
out vec3 worldCoordFar;

uniform mat4 invVP;

void main() {
  vec4 wcn = invVP*vec4(aPos.xy, 0.0, 1.0);
  vec4 wcf = invVP*vec4(aPos.xy, 1.0, 1.0);
  worldCoordNear = wcn.xyz / wcn.w;
  worldCoordFar = wcf.xyz / wcf.w;
  gl_Position = vec4(aPos, 1.0);
}