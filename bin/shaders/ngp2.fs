#version 330 core

layout (location = 0) out vec3 Normal;
layout (location = 1) out vec3 Albedo;
layout (location = 2) out vec3 DepthMetalRough;

in vec2 TexCoords;
in vec3 NormalWorld;
in vec3 PositionWorld;
uniform mat4 V;

uniform sampler2D texture_albedo;
uniform sampler2D texture_metal;
uniform sampler2D texture_rough;
uniform sampler2D texture_ao;

void main() {    
  Normal = normalize(NormalWorld);
  float AO = texture(texture_ao, TexCoords).r;
  Albedo = AO*texture(texture_albedo, TexCoords).rgb; // approximate
  DepthMetalRough.r = -(V*vec4(PositionWorld, 1.0)).z;
  DepthMetalRough.g = texture(texture_metal, TexCoords).r;
  DepthMetalRough.b = texture(texture_rough, TexCoords).r;
}