#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D AlbedoSpec;
uniform sampler2D WorldNormal;
uniform sampler2D WorldPosition;
uniform sampler2D SSAOInput;

uniform vec3 LightPos;
uniform vec3 CamPos;
uniform vec3 LightRadiance;

void main() {    
  vec4 albedoSpec = texture(AlbedoSpec, TexCoords);
  vec3 PositionWorld = texture(WorldPosition, TexCoords).rgb;
  vec3 NormalWorld = texture(WorldNormal, TexCoords).rgb;
  float Occulsion = texture(SSAOInput, TexCoords).r;

  vec3 toLight = normalize(LightPos - PositionWorld);
  vec3 toCam = normalize(CamPos - PositionWorld);
  vec3 halfV = normalize(toLight + toCam);

  float shinness = 2.2;
  float diff = max(0, dot(NormalWorld, toLight));
  float spec = pow(max(0, dot(halfV, NormalWorld)), shinness);

  vec3 ambient = albedoSpec.rgb*Occulsion;
  vec3 specular = spec*albedoSpec.aaa;

  FragColor = vec4(LightRadiance*(ambient*(1+diff)+specular), 1.0);
}