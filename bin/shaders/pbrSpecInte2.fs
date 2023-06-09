#version 330 core
out vec2 FragColor;

in vec2 TexCoords;

uniform int KernelSize;

const float PI = 3.14159265359;

/*D*/ 
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a      = roughness*roughness;
  float a2     = a*a;
  float NdotH  = max(dot(N, H), 0.0);
  float NdotH2 = NdotH*NdotH;

  float nom   = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}
/***/

/*G*/
float GeometrySchlickGGX(float NdotV, float roughness) {
  float a = roughness;
  float k = (a * a) / 2.0;

  float nom   = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2  = GeometrySchlickGGX(NdotV, roughness);
  float ggx1  = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}

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
  vec3 normal = vec3(0.0,0.0,1.0);
  float NdotWo = TexCoords.x;
  float Roughness = TexCoords.y;

  vec3 wo = vec3(sqrt(1.0-NdotWo*NdotWo), 0.0, NdotWo);
  
  float A = 0.0;
  float B = 0.0;
  for(int i = 0; i<KernelSize; i++) {
    vec2 Xi = Hammersley(uint(i), uint(KernelSize));
    vec3 wh = ImportanceSampleGGX(Xi, normal, Roughness);
    vec3 wi = normalize(dot(normal, wh)*2*wh - normal);

    float NdotWi = dot(normal, wi);
    if(NdotWi > 0.0) {
      float G = GeometrySmith(normal, wo, wi, Roughness);
      float WodotWh = dot(wo, wh);
      float deno = dot(wo, normal)*dot(wh, normal);
      float fc5 = pow(1-WodotWh, 5);
      A += G*WodotWh*(1-fc5) / deno;
      B += G*WodotWh*fc5 / deno;
    }
    
  }

  FragColor = vec2(A/KernelSize, B/KernelSize);
}