#version 330 core
out vec3 FragColor;

in vec3 textureDir;

uniform samplerCube EnvTex;
uniform sampler2D NoiseTex;

uniform float NoiseSize;
uniform int KernelSize;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits) {
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N) {
  return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}  

void main() {    
  vec3 noise = vec3(texture(NoiseTex, gl_FragCoord.xy/NoiseSize).xy, 0.0);

  vec3 normal = normalize(textureDir);
  vec3 tangent = normalize(cross(normal, noise));
  vec3 btangent = cross(normal, tangent);

  // use vec3 construct mat3, its fill mode is column major
  mat3 TBN = mat3(tangent, btangent, normal);

  vec3 DiffInteRes = vec3(0.0);
  for(int i = 0; i<KernelSize; i++) {
    vec2 rand = Hammersley(uint(i), uint(KernelSize));
    rand.y *= (2*PI); // cosTheta, Phi
    float sinTheta = sqrt(max(0.0, 1 - rand.x*rand.x));
    vec3 spt = vec3(sinTheta*cos(rand.y), sinTheta*sin(rand.y), rand.x);
    vec3 SampleDir = normalize(TBN*spt);
    vec3 Radiance = texture(EnvTex, SampleDir).rgb;
    DiffInteRes += Radiance*dot(SampleDir, normal);
  }
  vec3 res = DiffInteRes/KernelSize*(2*PI);
  FragColor = res;
}