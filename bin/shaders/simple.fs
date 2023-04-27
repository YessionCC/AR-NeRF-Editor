#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 sTexCoords;
in vec3 PositionWorld;

uniform vec3 LightPos;
uniform vec3 CamPos;
uniform vec3 LightRadiance;

//uniform sampler2D ShadowMap;
uniform samplerCube ShadowCubeMap;
uniform float LightPlaneFar;
uniform vec3 SampleDir3D[20];

void main() {    
  vec3 normal = vec3(0,1,0);
  vec3 toLight = normalize(LightPos - PositionWorld);
  vec3 toCam = normalize(CamPos - PositionWorld);
  vec3 halfV = normalize(toLight+toCam);

  float shinness = 5.2;
  float ambient = 0.1;
  float diff = max(0, dot(normal, toLight));
  float spec = pow(max(0, dot(halfV, normal)), shinness);

  /*
  float shadowZ = texture(ShadowMap, sTexCoords.xy).r;
  float shadow = shadowZ < sTexCoords.z ? 1.0 : 0.0;
  */

  float shadowDistance = texture(ShadowCubeMap, -toLight).r * LightPlaneFar;
  float posDistance = length(LightPos - PositionWorld);
  float rate = posDistance / shadowDistance;
  float sampleRadius = pow(rate, 2) * 0.04;

  float occ = 0.0;
  for(int i = 0; i<20; i++) {
    vec3 SampleWorldPos = PositionWorld + SampleDir3D[i] * sampleRadius;
    vec3 LightToPos = SampleWorldPos - LightPos;
    float shadowDistance = texture(ShadowCubeMap, LightToPos).r * LightPlaneFar;
    float posDistance = length(LightToPos);
    occ += shadowDistance < posDistance ? 1.0 : 0.0;
  }
  occ /= 20.0;
  
  //FragColor = vec4(sTexCoords.zzz, 1.0);
  FragColor = vec4(LightRadiance*(ambient+(diff+spec)*(1.0-occ)), 1.0);
}