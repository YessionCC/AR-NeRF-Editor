#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D WorldNormal;
uniform sampler2D WorldPosition;
uniform sampler2D NoiseTex;

uniform int KernelSize;
uniform float Radius;

uniform mat4 VP;
uniform mat4 V;

uniform vec2 ScreenSize;
uniform vec3 SamplePts[64];

void main() {    
  vec3 PositionWorld = texture(WorldPosition, TexCoords).xyz;
  vec3 PositionView = (V*vec4(PositionWorld, 1.0)).xyz;
  vec3 NormalWorld = texture(WorldNormal, TexCoords).xyz;

  // while we need let 4x4 texture fill the whole screen, so we need to scale coords
  // its a useful trick
  vec2 scale = ScreenSize / 4.0;
  vec3 noise = vec3(texture(NoiseTex, TexCoords*scale).xy, 0.0);

  // we use noise just make the tangent point randomly
  vec3 tangent = normalize(cross(NormalWorld, noise));
  vec3 btangent = cross(NormalWorld, tangent);

  // use vec3 construct mat3, its fill mode is column major
  mat3 TBN = mat3(tangent, btangent, NormalWorld);

  float occulsion = 0.0;
  for(int i = 0; i<KernelSize; i++) {
    vec3 SampleWorldPos = PositionWorld+Radius*TBN*SamplePts[i];
    vec4 SampleViewPos = V*vec4(SampleWorldPos, 1.0);
    vec4 SampleProjPos = VP*vec4(SampleWorldPos, 1.0);
    vec3 SampelNormPos = (SampleProjPos.xyz / SampleProjPos.w)*0.5+0.5;

    // SampleDepth is in View Space [-Near, -Far]
    // NOTICE!! because sample in view space has negative Z value !!!
    // below all in view space
    float SampleDepth = -texture(WorldPosition, SampelNormPos.xy).w;
    float rangeCheck = smoothstep(0.0, 1.0, Radius / abs(SampleDepth - PositionView.z));
    occulsion += (SampleDepth >= SampleViewPos.z ? 1.0:0.0)*rangeCheck;
  }
  FragColor = 1.0 - occulsion / KernelSize;
}