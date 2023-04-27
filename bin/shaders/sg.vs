#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

//layout (location = 3) in float aPCA_coeff[32];

out vec2 TexCoords;
out vec3 NormalWorld;
out vec3 PositionWorld;

//out float PCA_coeff[32];

uniform mat4 M;
uniform mat4 MVP;
uniform mat3 Normal2World_T;

void main() {
  TexCoords = aTexCoords;    
  NormalWorld = Normal2World_T*aNormal;
  PositionWorld = vec3(M*vec4(aPos, 1.0));
  /*
  for(int i = 0; i<32; i++) {
    PCA_coeff[i] = aPCA_coeff[i];
  }
  */

  gl_Position = MVP * vec4(aPos, 1.0);
}