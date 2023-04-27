#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.hpp"
#include "serializer.hpp"

class Scene;
class Shader;

class Camera { // now only perspective Cam
public:

  glm::vec3 WorldUp;

  glm::vec3 Position;
  glm::vec3 Front, Up, Right; // -z, y, x

  float nearZ = 0.1f;
  float farZ = 800.0f;
  float wDivH = 1.5f;
  float fov = 60; // in deg

private:
  glm::vec4 bkgColor;
  unsigned int renderTarget = 0;

public:
  Camera(
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
    glm::vec3 lookAtPoint = glm::vec3(0.0f, 0.0f, -1.0f), 
    glm::vec3 worldUp = glm::vec3(0.0f,1.0f,0.0f)
  );

  inline glm::mat4 GetViewMatrix() const {
    return glm::lookAt(Position, Position+Front, Up);
  }

  inline glm::mat4 GetProjectionMatrix() const {
    return glm::perspective(glm::radians(fov), wDivH, nearZ, farZ);
  }

  void SetBkgColor(glm::vec4 color) {
    bkgColor = color;
  }

  void render();
  void render(const Shader* shader, glm::vec4 bkgC=glm::vec4(0.0,0.0,0.0,1.0));

  inline void setRenderTarget(const RenderTexture* rt) {
    renderTarget = rt?rt->id:0;
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget);
  }

  static void SetCurrentCamera(Camera* cam);
  static Camera* CurrentCam(); 
};

class ViewportCamera: public Camera, public Serializer {
private:
  glm::vec3 focusPoint;
  float yaw, pitch; // relative to focusPoint in World space
  float distToFocus; // distance between focusPoint and Position

public:
  ViewportCamera(
    glm::vec3 position = glm::vec3(0.0f, 15.0f, 15.0f), 
    glm::vec3 focusPoint = glm::vec3(0.0f, 10.0f, 0.0f), 
    glm::vec3 worldUp = glm::vec3(0.0f,1.0f,0.0f)
  );

public:
  void updateVectors();
  // local (In local Up-Right coord)
  void translateInXYPlane(float dx, float dy);

  // world, relative to focusPoint (yaw: around Y axis(Up), pitch: around X axis(Right))
  // dYaw and dPitch are small value(do not exceed PI)
  // move relate to current position
  void rotateAroundFocus(float dYaw, float dPitch);

  void translateAlongZ(float dz);

  void printPositionAndLookDir() const;

  virtual void dump(std::ofstream& file);
  virtual void load(std::ifstream& file);
};