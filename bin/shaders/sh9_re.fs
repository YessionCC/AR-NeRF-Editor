#version 330 core
out vec3 FragColor;

in vec3 textureDir;
uniform vec3 sh9c[9];

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

#define CALC_FUNC(pp, func) res+=sh9c[pp]*func(dir)

void main() {
  vec3 res = vec3(0.0);
  vec3 dir = normalize(textureDir);
  CALC_FUNC(0, SH_00);
  CALC_FUNC(1, SH_1N1);
  CALC_FUNC(2, SH_10);
  CALC_FUNC(3, SH_11);
  CALC_FUNC(4, SH_2N2);
  CALC_FUNC(5, SH_2N1);
  CALC_FUNC(6, SH_20);
  CALC_FUNC(7, SH_21);
  CALC_FUNC(8, SH_22);
  FragColor = res;
}