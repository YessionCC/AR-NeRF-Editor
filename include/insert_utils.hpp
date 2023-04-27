#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

#include "libnpy/npy.hpp"

#include "scene.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "editor.hpp"
#include "texture.hpp"
#include "camera.hpp"

inline void SaveCurrentFrameRGB(int W, int H, std::string path) {
  std::vector<u_char> buffer(W*H*3);
  glReadPixels(0, 0, W, H, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
  unsigned long dims[3] = {H, W, 3};
  npy::SaveArrayAsNumpy(path, false, 3, dims, buffer);
}
inline void SaveCurrentFrameFloat3(int W, int H, std::string path) {
  std::vector<float> buffer(W*H*3);
  glReadPixels(0, 0, W, H, GL_RGB, GL_FLOAT, buffer.data());
  unsigned long dims[3] = {H, W, 3};
  npy::SaveArrayAsNumpy(path, false, 3, dims, buffer);
}
inline void SaveCurrentFrameFloat(int W, int H, std::string path) {
  std::vector<float> buffer(W*H);
  glReadPixels(0, 0, W, H, GL_RED, GL_FLOAT, buffer.data());
  unsigned long dims[3] = {H, W};
  npy::SaveArrayAsNumpy(path, false, 2, dims, buffer);
}
inline void SaveMat4(glm::mat4& mat, std::string path) {
  unsigned long dims[2] = {4, 4};
  // Notice !!!: glm::mat is column order!!
  npy::SaveArrayAsNumpy(path, true, 2, dims, (float*)&mat);
}

inline void GenProbe(std::vector<ModelBase*>& models, bool visualize = true) {
  int model_cnt = 0; 
  glm::vec3 models_center(0.0f);
  
  for(auto model: models) {
    Model* m = dynamic_cast<Model*>(model);
    if(!m) continue;
    models_center += m->getCenter_W();
    model_cnt ++;
  }
  models_center /= model_cnt;
  std::string cmd = 
    "cd /home/lofr/Projects/NeRF/nerf-pytorch/; python "
    "./run_nerf.py "
    "--config "
    "./configs/fern.txt "
    "--insert_obj False "
    "--generate_one_probe True "
    "--probe_position \""+Vec3Str(models_center)+"\"";
  if(visualize) {
    cmd += " --visualize_probes True";
  }
  std::cout<<"Exec cmd: "<<cmd<<std::endl;
  system(cmd.c_str());
  PrintVec3(models_center);
}

inline unsigned int ShowCameraView(
  ViewportCamera& cam, const Shader* shader, int W, int H) {
  static RenderTexture* ctex = new RenderTexture(W, H);
  if(W!=ctex->sizeW || H!=ctex->sizeH)
    ctex->resize(W, H);

  float wdivh_raw = cam.wDivH;
  cam.wDivH = 1.0f*W/H;
  glBindFramebuffer(GL_FRAMEBUFFER, ctex->id);

  glViewport(0,0,W,H);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  Scene::CurrentScene()->renderAll(shader);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  cam.wDivH = wdivh_raw;

  return ctex->textureColorbuffer;
}

inline void SaveInsertInfos(
  ViewportCamera& cam, std::vector<ModelBase*>& models, 
  int W, int H, std::string path) {
  static RenderTexture* tex = new RenderTexture(
    W, H, RenderTexture::NORMAL_NO_STENCIL, GL_RGB32F, GL_RGB, GL_FLOAT
  );
  if(W!=tex->sizeW || H!=tex->sizeH)
    tex->resize(W, H);

  float wdivh_raw = cam.wDivH;
  cam.wDivH = 1.0f*W/H;

  ModelBase* pcModel = nullptr;
  for(auto model: models) {
    Model* m = dynamic_cast<Model*>(model);
    if(!m) {
      pcModel = model;
      break;
    }
  }
  pcModel->mustUseSelfShader = true;
  Editor& editor = Editor::getInstance();
  editor.bkgColor = glm::vec3(0.0f);

  const Shader* renderShader = Shader::GetNormalShader(); 
  renderShader->use();
  renderShader->setBool("camcoord", false);
  renderShader->setBool("normalized", false);
  glBindFramebuffer(GL_FRAMEBUFFER, tex->id);
  glViewport(0,0,W,H);
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  Scene::CurrentScene()->renderAll(renderShader);
  SaveCurrentFrameFloat3(W, H, path+"/normal.npy");

  renderShader = Shader::GetDepthShader(); 
  renderShader->use();
  renderShader->setBool("camcoord", true);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  Scene::CurrentScene()->renderAll(renderShader);
  SaveCurrentFrameFloat3(W, H, path+"/depth.npy");
  pcModel->mustUseSelfShader = false;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  cam.wDivH = wdivh_raw;

  ShaderInfo& sInfo = Scene::CurrentScene()->GetCurrentShaderInfo();
  SaveMat4(sInfo.invV, path+"/cam_pose.npy");

  std::cout << "Render Mesh" << std::endl;
  std::string cmd = 
    "cd /home/lofr/Projects/NeRF/nerf-pytorch/; python "
    "./sh_render.py "
    "--not_show_im True "
    "--hdr_mapping False";
  std::cout<<"Exec cmd: "<<cmd<<std::endl;
  system(cmd.c_str());

  std::cout << "Rendering..." << std::endl;
  cmd = 
    "cd /home/lofr/Projects/NeRF/nerf-pytorch/; python "
    "./run_nerf.py "
    "--config "
    "./configs/fern.txt "
    "--insert_obj True ";
  std::cout<<"Exec cmd: "<<cmd<<std::endl;
  system(cmd.c_str());
}