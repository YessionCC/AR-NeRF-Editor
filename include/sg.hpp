#pragma once

#include <glm/glm.hpp>
#include "utility.hpp"
#include "shader.hpp"

#define MAX_SG_NUM 32

class SG {
public: 
  glm::vec3 amplitude;
  glm::vec3 axis;
  float sharpness;

public:
  SG(): amplitude(glm::vec3(.0f)), axis(glm::vec3(0.0f, 0.0f, 1.0f)), sharpness(.0f) {}
  SG(glm::vec3 amp, glm::vec3 axis, float sharp):
    amplitude(amp), axis(axis), sharpness(sharp) {}

  float integral_up_hemisphere() {
    float expTerm = 1.0f - exp(-1.0f * sharpness);
    return PI2  / sharpness * expTerm;
  }
  
  void setToShader(const Shader* s, std::string name) {
    s->setVec3(name+".Amplitude", amplitude);
    s->setVec3(name+".Axis", axis);
    s->setFloat(name+".Sharpness", sharpness);
  } 
  void setToShader(const Shader* s, std::string name, int idx) {
    std::string arrs = "["+std::to_string(idx)+"].";
    s->setVec3(name+arrs+"Amplitude", amplitude);
    s->setVec3(name+arrs+"Axis", axis);
    s->setFloat(name+arrs+"Sharpness", sharpness);
  } 
  /*
  void setToShaderVec4(const Shader* s, std::string name, int idx) {
    std::string arrs = "["+std::to_string(idx)+"]";
    s->setVec4(name+arrs, calc_to_shader_info());
  } 
  

private:
  glm::vec4 calc_to_shader_info() {
    float phi = glm::acos(axis.y);
    float theta = glm::atan(axis.z, axis.x); 
    float phi_n = phi / PI;
    float theta_n = theta / PI2 + 0.5;
    float lbd = (log10(sharpness) + 1) / 5;
    float inte = integral_up_hemisphere();
    return glm::vec4(phi_n, theta_n, lbd, inte);
  }
  */
};