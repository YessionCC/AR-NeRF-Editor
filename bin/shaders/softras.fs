#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D WorldNormal;
uniform sampler2D WorldPosition;

uniform vec3 LightPos;
uniform vec3 CamPos;
uniform vec3 LightRadiance;

uniform mat4 M;

void main() {    
  vec3 NormalWorld = texture(WorldNormal, TexCoords).rgb;
  FragColor = vec4(NormalWorld, 1.0f);
  /*
  vec3 PositionWorld = texture(WorldPosition, TexCoords).rgb;
  vec3 NormalWorld = texture(WorldNormal, TexCoords).rgb;

  PositionWorld = vec3(M*vec4(PositionWorld, 1.0f));
  NormalWorld = vec3(M*vec4(NormalWorld, 1.0f));

  vec3 toLight = normalize(LightPos - PositionWorld);
  vec3 toCam = normalize(CamPos - PositionWorld);
  vec3 halfV = normalize(toLight + toCam);

  float shinness = 2.2;
  float diff = max(0, dot(NormalWorld, toLight));
  float spec = pow(max(0, dot(halfV, NormalWorld)), shinness);

  FragColor = vec4(vec3(diff+spec), 1.0);
  */
}