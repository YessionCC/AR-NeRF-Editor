#pragma once

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <glm/glm.hpp>

#include "serializer.hpp"
#include "sg.hpp"

class PointCloud;
class Model;
class Editor: public Serializer {
public:
  int pipeline = 1; // 0 for SH, 1 for SG
  std::string currentModelPath;
  bool modifyWH = true;
  int viewportWH[2];
  int camportWH[2];
  glm::vec3 bkgColor;
  int drawMode = 0;
  int shadowMode = 1;
  bool sg_use_self_shadow = true;
  bool cam_coord_norm = false;
  bool cam_coord_depth = false;
  bool norm_normalized = true;
  glm::vec3 model_trans;
  glm::vec3 model_rotate;
  glm::vec3 model_center;
  float model_radius = 1.0f;
  float model_scale = 1.0f;
  float cam_fov = 60;
  unsigned int subCamport_texID = -1;

  float model_metal = 0.5;
  float model_rough = 0.5;
  glm::vec3 model_color;

  glm::vec3 main_dir_light;

  bool runCmp = false;
  bool saveRes = false;
  bool runProcess = false;
  int target_save_id = 0;
  bool simpleMaterial = true;

  bool consist_mode = false;
  int visualize_mode = 0;

  glm::vec3 trans_range_min, trans_range_max;
  glm::vec2 scale_range;

  glm::mat4 blender_trans;
  float blender_scale;

  glm::mat4 world_trans;

  SG lsgs[MAX_SG_NUM];

public:
  static Editor& getInstance() {
    static Editor editor;
    return editor;
  }
  void init(GLFWwindow* window, int w, int h);
  void initRange(PointCloud* pc, Model* model);
  void updateModelRange(Model* old, Model* _new);
  void update();
  void terminate();

  void drawMainLightDirection();

  glm::vec3 getObjTransInRawBlenderScene() const {
    return blender_trans * glm::vec4(blender_scale * model_trans, 1.0f);
  }

  void getCamTransInRawBlenderScene(glm::mat4& mat) const {
    mat = glm::inverse(world_trans) * mat; // view -> nerf coord
    mat[3] = glm::vec4(blender_scale * glm::vec3(mat[3]), 1.0f);
    mat = blender_trans * mat;
    //mat[3] = blender_trans * glm::vec4(blender_scale * glm::vec3(mat[3]), 1.0f);
    //mat[3] = glm::vec4(blender_scale * glm::vec3(blender_trans * mat[3]), 1.0f);
    //glm::mat3 rt = glm::mat3(blender_trans) * glm::mat3(mat);
    //mat[0] = glm::vec4(rt[0], 0.0f);
    //mat[1] = glm::vec4(rt[1], 0.0f);
    //mat[2] = glm::vec4(rt[2], 0.0f); // nerf coord -> raw blender
  }

  float getObjScaleInRawBlenderScene() const {
    return model_scale * blender_scale;
  }

  virtual void dump(std::ofstream& file);
  virtual void load(std::ifstream& file);

private:
  void editorDraw();

};
