#version 330 core
layout (location = 0) out vec3 oColor;
layout (location = 1) out vec3 oPosition;
layout (location = 2) out vec3 oNormal;
layout (location = 3) out float oDepth;


in vec2 TexCoords;
in vec3 NormalWorld;
in vec3 PositionWorld;

uniform vec3 LightPos;
uniform vec3 CamPos;
uniform vec3 LightRadiance;
uniform vec2 CamPlane;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

// after linearize z, the value is between NEAR and FAR
// equal to z in View Space
float LinearizeDepth(float depth) {
  float z = depth * 2.0 - 1.0;
  return (2.0 * CamPlane.x * CamPlane.y) / 
    (CamPlane.y + CamPlane.x - z * (CamPlane.y - CamPlane.x));    
}

void main() {    
  oNormal = normalize(NormalWorld);
  oPosition = PositionWorld;
  oDepth = gl_FragCoord.z;
  
  vec3 toLight = normalize(LightPos - PositionWorld);
  vec3 toCam = normalize(CamPos - PositionWorld);
  vec3 halfV = normalize(toLight + toCam);

  float shinness = 2.2;
  float diff = max(0, dot(NormalWorld, toLight));
  float spec = pow(max(0, dot(halfV, NormalWorld)), shinness);

  vec3 ambient = texture(texture_diffuse1, TexCoords).rgb;
  vec3 specular = spec*texture(texture_specular1, TexCoords).rrr;

  oColor = LightRadiance*(ambient*(1+diff)+specular);
}