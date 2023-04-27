#version 330 core
out vec4 NormalDepth;

in vec3 NormalWorld;
in vec3 PositionWorld;
uniform mat4 V;

void main() {    
  vec3 normal = normalize(NormalWorld);
  float depth = -(V*vec4(PositionWorld, 1.0)).z;
  NormalDepth.rgb = normal;
  NormalDepth.a = depth;
}