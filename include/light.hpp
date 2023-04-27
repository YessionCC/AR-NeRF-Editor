#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "texture.hpp"
#include "utility.hpp"

struct Shadow {
  int reX, reY;
  RenderTexture* rt;
  Shader* ss;
};

class Light {
public:
  enum Type {
    DirectionalLight,
    PointLight
  };

  Type ltType;
  glm::vec3 position;
  glm::vec3 radiance;
  bool isGenShadow = false;

protected:
  Shadow* shadow = nullptr;

public:
  Light(Type ltType, glm::vec3 pos, glm::vec3 L): 
    ltType(ltType), position(pos), radiance(L) {}

  virtual void initShadow(int reX, int reY) = 0;

  virtual void updateShadow() = 0;

  // set shadow receiver's shader params
  virtual void setShaderLightInfo(const Shader* ss) = 0;

  inline RenderTexture* getShadowRT() {return shadow->rt;}
};


class DirectionalLight: public Light {
public:
  glm::vec3 direction;
private:
  glm::mat4 world2LightView;
  glm::mat4 lightProj;
  glm::mat4 world2Proj;

public:
  DirectionalLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 L = glm::vec3(1,1,1)):
    Light(Type::DirectionalLight, pos, L), direction(dir) {
    updateMatrix();
    lightProj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.01f, 200.0f);
  }

  inline void updateMatrix() {
    world2LightView = glm::lookAt(position, position+direction, glm::vec3(0,1,0));
    world2Proj = lightProj*world2LightView;
  }

  void initShadow(int reX, int reY) override;

  void updateShadow() override;

  void setShaderLightInfo(const Shader* ss) override;
};


class PointLight: public Light {
private:
  glm::mat4 lightProj;
  glm::mat4 sixFacesTrans[6];

public:
  PointLight(glm::vec3 pos, glm::vec3 L = glm::vec3(1,1,1)): 
    Light(Type::PointLight, pos, L){}

  // NOTICE the order and worldUp directions!!
  inline void updateMatrix() {
    GetSixFacesTrans(position, lightProj, sixFacesTrans);
  }

  void initShadow(int reX, int reY) override;

  void updateShadow() override;

  void setShaderLightInfo(const Shader* ss) override;

};