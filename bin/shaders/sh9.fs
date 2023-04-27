#version 330 core
out vec3 FragColor;

uniform int KernelSize;
uniform samplerCube EnvTex;

const float PI = 3.14159265359;

float SH_00(vec3 v) {
  return 0.2820947918f;
}
float SH_1N1(vec3 v) {
  return 0.4886025119f*v.y;
}
float SH_10(vec3 v) {
  return 0.4886025119f*v.z;
}
float SH_11(vec3 v) {
  return 0.4886025119f*v.x;
}
float SH_2N2(vec3 v) {
  return 1.0925484306f*v.x*v.y;
}
float SH_2N1(vec3 v) {
  return 1.0925484306f*v.y*v.z;
}
float SH_20(vec3 v) {
  return 0.3153915653f*(3.0f*v.z*v.z-1);
}
float SH_21(vec3 v) {
  return 1.0925484306f*v.x*v.z;
}
float SH_22(vec3 v) {
  return 0.5462742153f*(v.x*v.x-v.y*v.y);
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

vec3 SampleSphere(vec2 Xi) {
  float cosTheta = Xi.x*2 - 1;
  float phi = Xi.y*2*PI;
  float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
  return vec3(sinTheta*cos(phi), sinTheta*sin(phi), cosTheta);
}

#define CALC_FUNC(pp, func) \
  if(gl_FragCoord.x == pp+0.5f) { \
    for(int i = 0; i<KernelSize; i++) { \
      vec2 Xi = Hammersley(uint(i), uint(KernelSize)); \
      vec3 v = SampleSphere(Xi); \
      vec3 val = texture(EnvTex, v).rgb; \
      res+=vec3(func(v)*val); \
    } \
  }

void main() {
  vec3 res = vec3(0.0);
  CALC_FUNC(0, SH_00);
  CALC_FUNC(1, SH_1N1);
  CALC_FUNC(2, SH_10);
  CALC_FUNC(3, SH_11);
  CALC_FUNC(4, SH_2N2);
  CALC_FUNC(5, SH_2N1);
  CALC_FUNC(6, SH_20);
  CALC_FUNC(7, SH_21);
  CALC_FUNC(8, SH_22);
  FragColor = res / KernelSize * (4*PI);
}