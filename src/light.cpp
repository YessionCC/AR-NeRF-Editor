#include "light.hpp"
#include "scene.hpp"
#include "utility.hpp"

using namespace std;

void DirectionalLight::initShadow(int reX, int reY) {
  isGenShadow = true;
  shadow = new Shadow;
  shadow->reX = reX; shadow->reY = reY;
  shadow->rt = new RenderTexture(
    reX, reY, RenderTexture::RTType::DEPTH_ONLY,
    GL_DEPTH_COMPONENT,
    GL_DEPTH_COMPONENT,
    GL_FLOAT
  );
  shadow->ss = new Shader(
    "./shaders/shadow.vs",
    "./shaders/shadow.fs"
  );
}

void DirectionalLight::updateShadow() {
  updateMatrix();
  vector<ModelBase*>& allMods = Scene::CurrentScene()->getAllModels();

  shadow->ss->use();
  glBindFramebuffer(GL_FRAMEBUFFER, shadow->rt->id);
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,shadow->reX, shadow->reY);
  for(ModelBase* m: allMods) {
    if(!m->generateShadow || m->mustUseSelfShader) continue;

    shadow->ss->setMat4("MVP", world2Proj * m->GetTransformMat());

    m->beforeDraw();
    m->draw(shadow->ss);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  shadow->ss->clear();
}

void DirectionalLight::setShaderLightInfo(const Shader* ss) {
  ss->setMat4("LightVP", world2Proj);
  // we set shadowmap and other special texture from 15
  ss->setRenderTexColorBuff("ShadowMap", shadow->rt, 15);
}

void PointLight::initShadow(int reX, int reY) {
  isGenShadow = true;
  lightProj = glm::perspective(glm::radians(90.0f), 1.0f*reX/reY, 0.01f, 200.0f);

  shadow = new Shadow;
  shadow->reX = reX; shadow->reY = reY;
  shadow->rt = new RenderTexture(
    reX, reY, RenderTexture::RTType::DEPTH_ONLY,
    GL_DEPTH_COMPONENT,
    GL_DEPTH_COMPONENT,
    GL_FLOAT, 0, true
  );
  shadow->ss = new Shader(
    "./shaders/pshadow.vs",
    "./shaders/pure.fs",
    {GL_DEPTH_TEST}, nullptr, nullptr,
    "./shaders/pshadow.gs"
  );
}

void PointLight::updateShadow() {
  updateMatrix();
  vector<ModelBase*>& allMods = Scene::CurrentScene()->getAllModels();

  glBindFramebuffer(GL_FRAMEBUFFER, shadow->rt->id);
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,shadow->reX, shadow->reY);

  shadow->ss->use();
  for(int i = 0; i<6; i++) {
    shadow->ss->setMat4("ShadowMatrices["+to_string(i)+"]", sixFacesTrans[i]);
  }
  shadow->ss->setVec3("LightPos", position);
  shadow->ss->setFloat("LightPlaneFar", 200.0f);
  
  for(ModelBase* m: allMods) {
    if(!m->generateShadow || m->mustUseSelfShader) continue;

    shadow->ss->setMat4("M", m->GetTransformMat());

    m->beforeDraw();
    m->draw(shadow->ss);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  shadow->ss->clear();

}

void PointLight::setShaderLightInfo(const Shader* ss) {
  // we set shadowmap and other special texture from 15
  ss->setFloat("LightPlaneFar", 200.0f);
  ss->setCubeMapRT("ShadowCubeMap", shadow->rt, 15);
  for(int i = 0; i<20; i++) {
    ss->setVec3("SampleDir3D["+to_string(i)+"]", sampleOffsetDirections[i]);
  }
}