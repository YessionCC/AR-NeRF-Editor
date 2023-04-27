#version 330 core
out vec3 FragColor;

in vec3 NormalWorld;

uniform mat4 V;
uniform bool normalized;
uniform bool camcoord;

void main() {    
  vec3 normal = normalize(NormalWorld);
  if(camcoord) {
    normal = mat3(V)*normal;
  }
  if(normalized) {
    normal = normal*0.5+0.5;
  }
  FragColor = normal;
}