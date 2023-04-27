#pragma once

#include<vector>

#include"model.hpp"
#include"light.hpp"
#include"specialModel.hpp"

class Camera;

struct ShaderInfo {
  glm::mat4 M, V, P, MV, VP, MVP, invV, invVP;
  glm::mat3 Normal2World, Normal2World_T, Normal2Cam;
  // CamPlane.x -> nearZ, CamPlane.y -> farZ
  // ScreenSize.x -> width, ScreenSize.y -> height
  glm::vec2 CamPlane, ScreenSize;
  glm::vec3 CamPos;
  Light* light;
  unsigned int ID;
};

class Scene {
public:
  Light* light = nullptr;
  // do not add skybox using addModel, just set it directly
  Skybox* skybox = nullptr;

private:
  std::vector<ModelBase*> models;

public:
  // call after all model added
  void Init();

  void addModel(ModelBase* model);

  // if set ss, then using this shader to render all
  // if set mType, only render one type model
  void renderAll(const Shader* ss=nullptr, ModelType mType = ModelType::Any) const;

  template<typename T>
  void getModelsByType(std::vector<T*>& ms, ModelType mType) const {
    ms.clear();
    for(ModelBase* model: models) {
      if(model->mType != mType) continue;
      ms.push_back(dynamic_cast<T*>(model));
    }
  }

  inline ModelBase* getModelWithID(int id) const {
    return models[id];
  } 

  inline std::vector<ModelBase*>& getAllModels() {return models;}

  static void UseScene(Scene* scene);
  static Scene* CurrentScene();

  // after any call renderAll, the shaderInfo will be updated
  static void SetBasicShaderParamsAll(const Shader* ss);
  static void SetBasicShaderParamsNoModel(const Shader* ss);
  static void UpdateNoModelInfo();
  static ShaderInfo& GetCurrentShaderInfo();
};