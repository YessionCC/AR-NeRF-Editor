#version 330 core
layout (location = 0) out vec4 AlbedoSpec;
layout (location = 1) out vec4 WorldPosition;
layout (location = 2) out vec3 WorldNormal;


in vec2 TexCoords;
in vec3 NormalWorld;
in vec3 PositionWorld;

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
  WorldNormal = normalize(NormalWorld);
  WorldPosition.xyz = PositionWorld;
  WorldPosition.w = LinearizeDepth(gl_FragCoord.z);
  AlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
  AlbedoSpec.a = texture(texture_specular1, TexCoords).r;
}