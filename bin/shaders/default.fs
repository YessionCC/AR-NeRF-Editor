#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 NormalWorld;
in vec3 PositionWorld;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform vec3 LightPos;
uniform vec3 CamPos;
uniform vec3 LightRadiance;

void main() {    
  vec3 normal = normalize(NormalWorld);
  vec3 toLight = normalize(LightPos - PositionWorld);
  vec3 toCam = normalize(CamPos - PositionWorld);
  vec3 halfV = normalize(toLight+toCam);

  float shinness = 2.2;
  float diff = max(0, dot(normal, toLight));
  float spec = pow(max(0, dot(halfV, normal)), shinness);

  vec3 ambient = texture(texture_diffuse1, TexCoords).rgb;
  vec3 specular = spec*texture(texture_specular1, TexCoords).rgb;

  FragColor = vec4(LightRadiance*(ambient*(1+diff)+specular), 1.0);
}