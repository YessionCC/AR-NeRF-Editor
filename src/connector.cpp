#include "connector.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <fstream>

#include "scene.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "editor.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "sg.hpp"
#include "pathManager.hpp"

NGPConnector* connector = nullptr;

NGPConnector* NGPConnector::getConnector() {
  return connector;
}
void NGPConnector::setConnector(NGPConnector* conn) {
  connector = conn;
}

// act + shadow mode + pos + rotaion
void NGPConnector::sendProbePos(std::vector<ModelBase*>& models) {
  static int len = sizeof(Action) + sizeof(int) + 
    sizeof(glm::vec3) + sizeof(glm::mat3);
  static Action act = Action::ProbePos;
  static char* buffer = new char[len];

  int model_cnt = 0; 
  glm::vec3 models_center(0.0f);
  
  for(auto model: models) {
    Model* m = dynamic_cast<Model*>(model);
    if(!m) continue;
    models_center += m->getCenter_W();
    model_cnt ++;
  }
  models_center /= model_cnt;
  Editor& editor = Editor::getInstance();
  editor.model_center = models_center;
  glm::mat3 rotation = glm::transpose(GetRotationMatrix(editor.model_rotate));

  int64_t idx_len = 0; 
  memcpy(buffer + idx_len, &act, sizeof(Action)); 
  idx_len += sizeof(Action);
  memcpy(buffer + idx_len, &editor.shadowMode, sizeof(int)); 
  idx_len += sizeof(int);
  memcpy(buffer + idx_len, &models_center, sizeof(glm::vec3)); 
  idx_len += sizeof(glm::vec3);
  memcpy(buffer+idx_len, &rotation, sizeof(glm::mat3));
  
  conn.Send(buffer, len);
} 


void NGPConnector::sendCamPose() {
  static int len = sizeof(Action) + sizeof(glm::mat4);
  static Action act = Action::CamPose;
  static char* buffer = new char[len];
  ShaderInfo& sInfo = Scene::CurrentScene()->GetCurrentShaderInfo();
  glm::mat4 world_trans = Editor::getInstance().world_trans;
  glm::mat4 invV = glm::transpose(glm::inverse(world_trans) * sInfo.invV);

  memcpy(buffer, &act, sizeof(Action));
  memcpy(buffer+sizeof(Action), &invV, sizeof(glm::mat4));
  
  conn.Send(buffer, len);
}

// act model_radius bMin bMax (normal depth) SH, (color depth) SG
void NGPConnector::sendMapsSimple(
  ViewportCamera& cam, std::vector<ModelBase*>& models, 
  int W, int H, bool use_sg_shader) {
  static Action act = Action::Maps;

  static RenderTexture* tex = new RenderTexture(
    W, H, RenderTexture::NORMAL_NO_STENCIL, GL_RGBA32F, GL_RGBA, GL_FLOAT
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

  const Shader* renderShader = 
    use_sg_shader ? Shader::GetSGShader(): Shader::GetNGP1Shader(); 
  renderShader->use();
  glBindFramebuffer(GL_FRAMEBUFFER, tex->id);
  glViewport(0,0,W,H);
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_CULL_FACE);
  Scene::CurrentScene()->renderAll(renderShader);
  glDisable(GL_CULL_FACE);

  std::vector<Model*> model;
  Scene::CurrentScene()->getModelsByType(model, ModelType::Meshes);
  float scale = editor.model_scale;
  // note: model.gerRadius will return the max axis length of the bbox3
  editor.model_radius = model[0]->getRadius() / 2 * scale;
  glm::mat4 VP = cam.GetProjectionMatrix() * cam.GetViewMatrix();
  glm::ivec2 bMin, bMax;
  model[0]->getScreenBounding(bMin, bMax, H, W, VP);
  glm::ivec2 bArea = bMax - bMin;

  char* buffer = new char[
    sizeof(Action)+sizeof(float)+sizeof(glm::ivec2)*2+
    bArea.x*bArea.y*4*sizeof(float)
  ];
  int64_t idx_len = 0; 
  memcpy(buffer+idx_len, &act, sizeof(Action));
  idx_len+=sizeof(Action);
  memcpy(buffer+idx_len, &editor.model_radius, sizeof(float));
  idx_len+=sizeof(float);
  memcpy(buffer+idx_len, &bMin, sizeof(glm::ivec2));
  idx_len+=sizeof(glm::ivec2);
  memcpy(buffer+idx_len, &bMax, sizeof(glm::ivec2));
  idx_len+=sizeof(glm::ivec2);

  //glReadPixels(0, 0, W, H, GL_RGBA, GL_FLOAT, buffer+idx_len);
  glReadPixels(bMin.y, H - bMin.x - bArea.x, bArea.y, bArea.x, GL_RGBA, GL_FLOAT, buffer+idx_len);
  idx_len += bArea.x*bArea.y*4*sizeof(float);

  pcModel->mustUseSelfShader = false;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  cam.wDivH = wdivh_raw;

  conn.Send(buffer, idx_len);
  delete[] buffer;
}

// act H W normal depth albedo rough and metal
void NGPConnector::sendMapsComplex(
  ViewportCamera& cam, std::vector<ModelBase*>& models, int W, int H) {
  static Action act = Action::Maps;

  static MultiRenderTexture* tex = new MultiRenderTexture(W, H);
  static bool texInit = false;

  if(!texInit) {
    texInit = true;
    tex->startAdd();
    tex->addTarget(RenderTexture::RTType::COLOR_ONLY, GL_RGB32F, GL_RGB, GL_FLOAT, 0);
    tex->addTarget(RenderTexture::RTType::COLOR_ONLY, GL_RGB32F, GL_RGB, GL_FLOAT, 1);
    tex->addTarget(RenderTexture::RTType::COLOR_ONLY, GL_RGB32F, GL_RGB, GL_FLOAT, 2);
    tex->completeAdd();
    tex->addDepthOrStencil();
    tex->checkAndCompleteAll();
  }

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

  const Shader* renderShader = Shader::GetNGP2Shader(); 
  renderShader->use();

  glBindFramebuffer(GL_FRAMEBUFFER, tex->id);
  glViewport(0,0,W,H);
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_CULL_FACE);
  Scene::CurrentScene()->renderAll(renderShader);
  glDisable(GL_CULL_FACE);

  std::vector<Model*> model;
  Scene::CurrentScene()->getModelsByType(model, ModelType::Meshes);
  float scale = editor.model_scale;
  // note: model.gerRadius will return the max axis length of the bbox3
  editor.model_radius = model[0]->getRadius() / 2 * scale;
  glm::mat4 VP = cam.GetProjectionMatrix() * cam.GetViewMatrix();
  glm::ivec2 bMin, bMax;
  model[0]->getScreenBounding(bMin, bMax, H, W, VP);
  glm::ivec2 bArea = bMax - bMin;

  char* buffer = new char[
    sizeof(Action)+sizeof(float)+sizeof(glm::ivec2)*2+
    bArea.x*bArea.y*9*sizeof(float)
  ];
  int64_t idx_len = 0; 
  memcpy(buffer+idx_len, &act, sizeof(Action));
  idx_len+=sizeof(Action);
  memcpy(buffer+idx_len, &editor.model_radius, sizeof(float));
  idx_len+=sizeof(float);
  memcpy(buffer+idx_len, &bMin, sizeof(glm::ivec2));
  idx_len+=sizeof(glm::ivec2);
  memcpy(buffer+idx_len, &bMax, sizeof(glm::ivec2));
  idx_len+=sizeof(glm::ivec2);

  // glReadBuffer can set which attachment can be read
  for(int i = 0; i<3; i++) {
    glReadBuffer(GL_COLOR_ATTACHMENT0+i);
    glReadPixels(bMin.y, H - bMin.x - bArea.x, bArea.y, bArea.x, GL_RGB, GL_FLOAT, buffer+idx_len);
    idx_len += bArea.x*bArea.y*3*sizeof(float);
  }

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  pcModel->mustUseSelfShader = false;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  cam.wDivH = wdivh_raw;

  conn.Send(buffer, idx_len);
  delete[] buffer;
}

void NGPConnector::sendRenderCall() {
  static int len = sizeof(Action);
  static Action act = Action::Render;
  static char* buffer = new char[len];
  memcpy(buffer, &act, len);
  conn.Send(buffer, len);
}

// just send one empty info to tell server save the result image
void NGPConnector::sendRenderSaveCall(SavePack* sp) {
  static Action act = Action::Render;
  char* buffer = new char[sizeof(Action)+sizeof(int)+sp->save_prefix.size()];
  int sff = sp->is_save_info?1:0;
  memcpy(buffer, &act, sizeof(Action));
  memcpy(buffer+sizeof(Action), &sff, sizeof(int));
  memcpy(buffer+sizeof(Action)+sizeof(int), sp->save_prefix.c_str(), sp->save_prefix.size());
  conn.Send(buffer, sizeof(Action)+sizeof(int)+sp->save_prefix.size());
}

void NGPConnector::sendMaterial() {
  static int len = sizeof(Action) + 2*sizeof(float) + sizeof(glm::vec3);
  static Action act = Action::Material;
  static char* buffer = new char[len];
  Editor& editor = Editor::getInstance();
  memcpy(buffer, &act, sizeof(Action));
  memcpy(buffer+sizeof(Action), &editor.model_rough, sizeof(float));
  memcpy(buffer+sizeof(Action)+sizeof(float), &editor.model_metal, sizeof(float));
  memcpy(buffer+sizeof(Action)+sizeof(float)*2, &editor.model_color, sizeof(glm::vec3));
  conn.Send(buffer, len);
}

// only support one model /// deprecated
void NGPConnector::sendShadowInfo(ViewportCamera& cam, int H, int W) {
  std::vector<Model*> model;
  Scene::CurrentScene()->getModelsByType(model, ModelType::Meshes);
  float scale = Editor::getInstance().model_scale;
  // note: model.gerRadius will return the max axis length of the bbox3
  float model_radius = model[0]->getRadius() / 2 * scale;
  glm::ivec2 bMin, bMax;
  float wdivh_raw = cam.wDivH;
  cam.wDivH = 1.0f*W/H;
  glm::mat4 VP = cam.GetProjectionMatrix() * cam.GetViewMatrix();
  cam.wDivH = wdivh_raw;
  model[0]->getScreenBounding(bMin, bMax, H, W, VP);
  //printf("min: %d %d\n", bMin.x, bMin.y);//
  //printf("max: %d %d\n", bMax.x, bMax.y);//

  static int len = sizeof(Action) + sizeof(float) + sizeof(glm::ivec2)*2;
  static Action act = Action::ShadowField;
  static char* buffer = new char[len];
  memcpy(buffer, &act, sizeof(Action));
  memcpy(buffer+sizeof(Action), &model_radius, sizeof(float));
  memcpy(buffer+sizeof(Action)+sizeof(float), &bMin, sizeof(glm::ivec2));
  memcpy(buffer+sizeof(Action)+sizeof(float)+sizeof(glm::ivec2), 
    &bMax, sizeof(glm::ivec2));
  conn.Send(buffer, len);
}

// Send shadow field file path and vol_range and grid_size
void NGPConnector::sendShadowFieldInfo(std::string model_path) {
  std::string model_name = GetModelNameFromPath(model_path);
  std::string sf_path = "./sf/results/"+model_name+".txt";
  std::ifstream sf_f(sf_path);
  if(!sf_f.good()) { // if never run sf to this model
    printf("Insert new object, precomputing...\n");
    std::string cmd_str = "./sf/sf "+model_path+" "+sf_path;
    printf("Run CMD: %s\n", cmd_str.c_str());
    system(cmd_str.c_str());
  }
  sf_f.close();
  static Action act = Action::ShadowFieldPath;
  int len = sizeof(Action) + model_name.size();
  char* buffer = new char[len];
  memcpy(buffer, &act, sizeof(Action));
  memcpy(buffer+sizeof(Action), model_name.c_str(), model_name.size());
  conn.Send(buffer, len);
}
// Send ssdf file path and vol_range and grid_size
void NGPConnector::sendSSDFInfo(std::string model_path) {
  std::string model_name = GetModelNameFromPath(model_path);
  std::string sg_path = "./sg/results/"+model_name+".tar";
  std::ifstream sg_f(sg_path);
  if(!sg_f.good()) { // if never run sf to this model
    printf("Insert new object, precomputing...\n");
    std::string cmd_str = "python3 ./sg/ssdf.py &";
    sleep(3);
    printf("Run CMD: %s\n", cmd_str.c_str());
    system(cmd_str.c_str());
    cmd_str = "./sg/sg "+model_path+" "+sg_path+" 2 20";
    printf("Run CMD: %s\n", cmd_str.c_str());
    system(cmd_str.c_str());
  }
  sg_f.close();
  static Action act = Action::SSDFPath;
  int len = sizeof(Action) + model_name.size();
  char* buffer = new char[len];
  memcpy(buffer, &act, sizeof(Action));
  memcpy(buffer+sizeof(Action), model_name.c_str(), model_name.size());
  conn.Send(buffer, len);
}
void NGPConnector::sendSForSSDFInfo(std::string model_path) {
  Editor::getInstance().pipeline == 0 ?
    sendShadowFieldInfo(model_path):
    sendSSDFInfo(model_path);
}

void NGPConnector::sendShadowMap() {
  std::vector<Model*> model;
  Scene::CurrentScene()->getModelsByType(model, ModelType::Meshes);
  Editor& editor = Editor::getInstance();
  float r = 1.2*editor.model_radius;
  glm::vec3 s_cam_o = editor.model_center + r * editor.main_dir_light;
  //s_cam_o = editor.world_trans * glm::vec4(s_cam_o, 1.0f);
  glm::vec3 s_cam_d =  -editor.main_dir_light;
  //- editor.world_trans * glm::vec4(editor.main_dir_light, 1.0f);
  glm::mat4 v_mat = glm::lookAt(s_cam_o, s_cam_o + s_cam_d, glm::vec3(0.0,1.0,0.0));
  glm::mat4 p_mat = glm::ortho(-r, r, -r, r, 0.0f, r*2);
  glm::mat4 M = model[0]->GetTransformMat();
  glm::mat4 VP = glm::transpose(p_mat * v_mat);
  const Shader* ds = Shader::GetSDepthShader();
  ds->use();
  ds->setMat4("MVP", p_mat*v_mat* M);
  //ds->setMat4("V", v_mat);
  //ds->setMat4("M", M);

  const int texSize = 100;
  static RenderTexture* tex = new RenderTexture(
    texSize, texSize, RenderTexture::NORMAL_NO_STENCIL, GL_R32F, GL_RED, GL_FLOAT
  );
  glBindFramebuffer(GL_FRAMEBUFFER, tex->id);
  glViewport(0,0,texSize,texSize);
  glClearColor(10000,1,1,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  model[0]->draw(ds);

  static char* buffer = new char[
    sizeof(Action)+sizeof(int)+sizeof(glm::mat4)+
    texSize*texSize*sizeof(float)
  ];
  static Action act = Action::ShadowMap;
  int64_t idx_len = 0; 
  memcpy(buffer+idx_len, &act, sizeof(Action));
  idx_len+=sizeof(Action);
  memcpy(buffer+idx_len, &texSize, sizeof(int));
  idx_len+=sizeof(int);
  memcpy(buffer+idx_len, &VP, sizeof(glm::mat4));
  idx_len+=sizeof(glm::mat4);
  glReadPixels(0,0,texSize,texSize,GL_RED,GL_FLOAT,buffer+idx_len);
  idx_len+=sizeof(float)*texSize*texSize;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0,0,0,0);
  conn.Send(buffer, idx_len);
}

void NGPConnector::sendSGUseSShadow() {
  static Action act = Action::SG_Use_SShadow;
  static char* buf = new char[sizeof(Action)+sizeof(int)];
  int use_sg_ss = Editor::getInstance().sg_use_self_shadow;
  memcpy(buf, &act, sizeof(Action));
  memcpy(buf+sizeof(Action), &use_sg_ss, sizeof(int));
  conn.Send(buf, sizeof(Action)+sizeof(int));
}

void NGPConnector::sendMethodsCompareInfo() {
  static Action act = Action::Cmp_Methods;
  static char* buf = new char[sizeof(Action)];
  std::string IRAdobe_path = PathMan::getIns().IRAdobe_base;
  std::string result_path = PathMan::getIns().ResultPath();
  std::string _cmd = "sh "+IRAdobe_path+"runReal20.sh "+
    PathMan::getIns().GetEnvImResultPath()+ " " + 
    IRAdobe_path + "results/";
  std::cout<<"Run cmd: "<<_cmd<<std::endl;
  system(_cmd.c_str());
  memcpy(buf, &act, sizeof(Action));
  conn.Send(buf, sizeof(Action));
}

void NGPConnector::sendProcessDecompositeCmpCall() {
  static Action act = Action::Decomposition_Cmp;
  static char* buf = new char[sizeof(Action)];
  memcpy(buf, &act, sizeof(Action));
  conn.Send(buf, sizeof(Action));
}

void NGPConnector::sendUpdateIndexCall() {
  static Action act = Action::UpdateSaveIndex;
  static char* buf = new char[sizeof(Action)+sizeof(int)];
  memcpy(buf, &act, sizeof(Action));
  memcpy(buf+sizeof(Action), &PathMan::getIns().currentSaveID, sizeof(int));
  conn.Send(buf, sizeof(Action)+sizeof(int));
}


void NGPConnector::recvHWF() {
  char* buf = new char[sizeof(int)*2+sizeof(float)];
  conn.Receive(buf);
  Editor& editor = Editor::getInstance();
  editor.camportWH[1] = *(int*)buf;
  editor.camportWH[0] = *(int*)(buf+sizeof(int));
  float focal = *(float*)(buf+sizeof(int)*2);
  editor.cam_fov = 2*glm::atan(editor.camportWH[1] / (2.0*focal));
  std::cout<<"Receives HWF: "<< editor.camportWH[1]
    << " " << editor.camportWH[0] << " " << focal << std::endl;
}

void NGPConnector::recvMainDirectionLight() {
  Editor& editor = Editor::getInstance();
  conn.Receive((char*)&editor.main_dir_light);
  sendShadowMap();
}

void NGPConnector::recvBlenderTrans() {
  glm::mat4 trans; float scale;
  conn.Receive((char*)&trans);
  conn.Receive((char*)&scale);
  Editor& editor = Editor::getInstance();
  editor.blender_trans = glm::transpose(trans);
  editor.blender_scale = scale;
}

void NGPConnector::recvRenderCompleteFlag() {
  int flag;
  conn.Receive((char*)&flag);
}

void NGPConnector::recvSGLights() {
  static float lsgs[MAX_SG_NUM][7];
  conn.Receive((char*)lsgs);
  Editor& editor = Editor::getInstance();
  SG* sgs = editor.lsgs;
  for(int i = 0; i<MAX_SG_NUM; i++) {
    sgs[i].axis = glm::vec3(lsgs[i][0], lsgs[i][1], lsgs[i][2]);
    sgs[i].axis = editor.world_trans * glm::vec4(sgs[i].axis, 1.0f);
    sgs[i].sharpness = lsgs[i][3];
    sgs[i].amplitude = glm::vec3(lsgs[i][4], lsgs[i][5], lsgs[i][6]);
  }
}