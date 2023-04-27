#include "scene.hpp"

#include <algorithm>

#include "texture.hpp"
#include "camera.hpp"
#include "viewport.hpp"
#include "editor.hpp"

using namespace std;

Scene* currentScene = nullptr;
ShaderInfo curShaderInfo;

void Scene::UseScene(Scene* scene) {
  currentScene = scene;
}
Scene* Scene::CurrentScene() {
  return currentScene;
}

void Scene::addModel(ModelBase* model) {
  models.push_back(model);
}

void Scene::Init() {
  std::sort(models.begin(), models.end(), [](ModelBase* l, ModelBase* r){
    return *l<*r;
  });
  for(unsigned int i = 0; i<models.size(); i++) {
    models[i]->ID_scene = i;
  }
  if(skybox)
    models.push_back(skybox);
  curShaderInfo.light = light;
}

void Scene::SetBasicShaderParamsAll(const Shader* shader) {
  SetBasicShaderParamsNoModel(shader);
  shader->setMat4("M", curShaderInfo.M);
  shader->setMat4("MV", curShaderInfo.MV);
  shader->setMat4("MVP", curShaderInfo.MVP);
  shader->setMat3("Normal2World", curShaderInfo.Normal2World);
  shader->setMat3("Normal2World_T", curShaderInfo.Normal2World_T);
  shader->setMat3("Normal2Cam", curShaderInfo.Normal2Cam);
  shader->setUInt("ID", curShaderInfo.ID);
}

void Scene::SetBasicShaderParamsNoModel(const Shader* shader) {
  shader->setMat4("V", curShaderInfo.V);
  shader->setMat4("P", curShaderInfo.P);
  shader->setMat4("VP", curShaderInfo.VP);
  shader->setMat4("invV", curShaderInfo.invV);
  shader->setMat4("invVP", curShaderInfo.invVP);

  shader->setVec3("CamPos", curShaderInfo.CamPos);
  shader->setVec2("CamPlane", curShaderInfo.CamPlane);
  shader->setVec2("ScreenSize", curShaderInfo.ScreenSize);

  if(curShaderInfo.light) {
    shader->setVec3("LightPos", curShaderInfo.light->position);
    shader->setVec3("LightRadiance", curShaderInfo.light->radiance);
  }
}

ShaderInfo& Scene::GetCurrentShaderInfo() {
  return curShaderInfo;
}

// when not call render all but need new data, call it
void Scene::UpdateNoModelInfo() {
  Camera* cam = Camera::CurrentCam();
  Viewport& vp = Viewport::GetInstance();
  curShaderInfo.P = cam->GetProjectionMatrix();
  curShaderInfo.V = cam->GetViewMatrix();

  curShaderInfo.VP = curShaderInfo.P*curShaderInfo.V;
  curShaderInfo.invV = glm::inverse(curShaderInfo.V);
  curShaderInfo.invVP = glm::inverse(curShaderInfo.VP);

  curShaderInfo.CamPos = cam->Position;
  curShaderInfo.CamPlane = glm::vec2(cam->nearZ, cam->farZ);
  curShaderInfo.ScreenSize = glm::vec2(vp.GetWidth(), vp.GetHeight());
}

void Scene::renderAll(const Shader* ss, ModelType mType) const {
  UpdateNoModelInfo();

  bool setModelType = mType != ModelType::Any;

  const Shader* shader = ss;
  if(shader) shader->use();

  glm::mat4 world_trans = Editor::getInstance().world_trans;

  for(unsigned int i = 0; i<models.size(); ++i) {
    ModelBase* model = models[i];

    if(setModelType && mType != model->mType) continue;
    if(ss && model->mustUseSelfShader) continue;
    if(!ss && shader != model->shader) {
      if(shader) shader->clear();
      shader = model->shader;
      shader->use();

      /* 
        deal with shadow, we assume that if a receiver use a certain shader,
        than all model using this shader is receiver
      */
      if(light && light->isGenShadow && model->receiveShadow) {
        light->setShaderLightInfo(shader);
      }
    }

    curShaderInfo.M = world_trans * model->GetTransformMat();
    curShaderInfo.MV = curShaderInfo.V*curShaderInfo.M;
    curShaderInfo.MVP = curShaderInfo.VP*curShaderInfo.M;
    curShaderInfo.Normal2World = glm::mat3(glm::transpose(glm::inverse(model->GetTransformMat())));
    // Normal2World_T is the normal transform to real render coordination(after world trans)
    curShaderInfo.Normal2World_T = glm::mat3(glm::transpose(glm::inverse(curShaderInfo.M)));
    curShaderInfo.Normal2Cam = glm::mat3(curShaderInfo.V)*curShaderInfo.Normal2World;
    curShaderInfo.ID = i+1; // NOTICE: ID num from 1

    SetBasicShaderParamsAll(shader);

    if(model->customDraw) {
      model->customDraw(model, shader);
    }
    else {
      model->curShader = shader;
      model->beforeDraw();
      model->draw(shader);
    }
  }
  if(shader) shader->clear();
}