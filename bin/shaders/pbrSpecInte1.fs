#version 330 core
out vec3 FragColor;

in vec3 textureDir;

uniform samplerCube EnvTex;

uniform float Roughness;
uniform int KernelSize;

const float PI = 3.14159265359;


vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
  float a = roughness*roughness;

  float phi = 2.0 * PI * Xi.x;
  float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
  float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

  // from spherical coordinates to cartesian coordinates
  vec3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;

  // from tangent-space vector to world-space sample vector
  vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent   = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);

  vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
  return normalize(sampleVec);
}

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
  float totWeight = 0.0;
  vec3 pcol = vec3(0.0);
  vec3 normal = normalize(textureDir);
  for(int i = 0; i<KernelSize; i++) {
    vec2 Xi = Hammersley(uint(i), uint(KernelSize));
    vec3 wh = ImportanceSampleGGX(Xi, normal, Roughness);
    vec3 wi = normalize(dot(normal, wh)*2*wh - normal);

    float cosThetaI = dot(wi, normal);
    if(cosThetaI > 0.0) {
      pcol += texture(EnvTex, wi).rgb*cosThetaI;
      totWeight += cosThetaI;
    }
  }
  FragColor = pcol / totWeight;
  
}