#version 330 core
out float FragColor;

in vec2 TexCoords;
uniform samplerCube EnvTex;

const float PI = 3.14159265359;

void main() {
  float phi = TexCoords.y * PI;
  float theta = (TexCoords.x - 0.5) * 2 * PI;
  float sinPhi = sin(phi);
  vec3 dir = vec3(cos(theta)*sinPhi, cos(phi), sin(theta)*sinPhi);
  FragColor = texture(EnvTex, dir).r;
}