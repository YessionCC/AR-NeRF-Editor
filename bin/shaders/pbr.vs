#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out vec2 TexCoords;
out vec3 NormalWorld;
out vec3 TangentWorld;
out vec3 PositionWorld;

uniform mat4 M;
uniform mat4 MVP;
uniform mat3 Normal2World;

void main() {
  TexCoords = aTexCoords;    
  NormalWorld = Normal2World*aNormal;
  TangentWorld = Normal2World*aTangent;
  PositionWorld = vec3(M*vec4(aPos, 1.0));
  gl_Position = MVP * vec4(aPos, 1.0);
}