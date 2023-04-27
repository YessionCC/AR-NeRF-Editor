#include "editor.hpp"
#include "utility.hpp"
#include "insert_utils.hpp"
#include "model.hpp"
#include "connector.hpp"
#include "pathManager.hpp"

void Editor::init(GLFWwindow* window, int w, int h) {
  ImGui::CreateContext();     // Setup Dear ImGui context
  ImGui::StyleColorsDark();       // Setup Dear ImGui style
  ImGui_ImplGlfw_InitForOpenGL(window, true);     // Setup Platform/Renderer backends
  ImGui_ImplOpenGL3_Init("#version 330");
  viewportWH[0] = w; viewportWH[1] = h;
  camportWH[0] = w/4; camportWH[1] = h/4;
  bkgColor = glm::vec3(0);
  trans_range_min = glm::vec3(-20);
  trans_range_max = glm::vec3(20);
  scale_range = glm::vec2(0.01, 10);
  world_trans = glm::mat4(1.0f);
  model_color = glm::vec3(1.0,1.0,1.0);
}

void Editor::initRange(PointCloud* pc, Model* model) {
  glm::vec3 pcMin, pcMax;
  pc->getBound(pcMin, pcMax);
  glm::vec3 pcBnd = pcMax - pcMin;
  float pc_rad = std::max({pcBnd.x, pcBnd.y, pcBnd.z});
  glm::vec3 model_c = model->getCenter_W();
  float model_rad = model->getRadius();
  float scale_b = pc_rad / model_rad;
  scale_range = glm::vec2(scale_b*0.05, scale_b*2);
  trans_range_min = pcMin + 0.5f*(pcMin - pcMax);
  trans_range_max = pcMax + 0.5f*(pcMax - pcMin);
  model_trans = 0.5f*(pcMin+pcMax);
  model_scale = scale_b * 0.4;
}

// mainly adjust scale range
void Editor::updateModelRange(Model* old, Model* _new) {
  float old_rad = old->getRadius();
  float new_rad = _new->getRadius();
  float rate = old_rad / new_rad;
  model_scale *= rate;
  scale_range *= rate;
}

void Editor::update() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  editorDraw();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::terminate() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Editor::dump(std::ofstream& file) {
  file << "Editor" << std::endl;
  file << "FOV "<<cam_fov << std::endl;
  file << "Viewport_WH "<<viewportWH[0]<<" "<<viewportWH[1] << std::endl;
  file << "Camport_WH "<<camportWH[0]<<" "<<camportWH[1] << std::endl;
  file << "bkgColor "<<Vec3Str(bkgColor) << std::endl;
  file << "mTrans "<<Vec3Str(model_trans) << std::endl;
  file << "mRot "<<Vec3Str(model_rotate) << std::endl;
  file << "mScale "<<model_scale << std::endl;
  file << "wTrans "<<Mat3Str(world_trans) << std::endl; 
  file << "rough "<< model_rough << std::endl; 
  file << "metal "<< model_metal; 
}

void Editor::load(std::ifstream& file) {
  std::string cc; 
  file >> cc;
  file >> cc >> cam_fov;
  file >> cc >> viewportWH[0] >> viewportWH[1];
  file >> cc >> camportWH[0] >> camportWH[1];
  file >> cc >> bkgColor.r >> bkgColor.g >> bkgColor.b;
  file >> cc >> model_trans.x >> model_trans.y >> model_trans.z;
  file >> cc >> model_rotate.x >> model_rotate.y >> model_rotate.z;
  file >> cc >> model_scale;
  file >> cc >> 
    world_trans[0][0] >> world_trans[0][1] >> world_trans[0][2] >>
    world_trans[1][0] >> world_trans[1][1] >> world_trans[1][2] >>
    world_trans[2][0] >> world_trans[2][1] >> world_trans[2][2];
  file >> cc >> model_rough;
  file >> cc >> model_metal;
}

void Editor::drawMainLightDirection() {
  auto dl = ImGui::GetBackgroundDrawList();
  glm::ivec2 c_ras1, c_ras2;
  const glm::mat4& proj = Scene::GetCurrentShaderInfo().VP;
  glm::vec3 orgin = world_trans * glm::vec4(model_center, 1.0);
  glm::vec3 target = world_trans * glm::vec4(model_center+main_dir_light, 1.0);
  GetScreenPos(orgin, proj, viewportWH[1], viewportWH[0], c_ras1);
  GetScreenPos(target, proj, viewportWH[1], viewportWH[0], c_ras2);
  //c_ras2 = (c_ras2 - c_ras1)*10 + c_ras1;
  ImVec2 cc1(c_ras1.y, c_ras1.x);
  ImVec2 cc2(c_ras2.y, c_ras2.x);
  //printf("%d %d\n", c_ras1.x, c_ras1.y);
  dl->AddLine(cc1, cc2, ImColor(1.0f,0.0f,0.0f), 5);

  ImVec2 statusPos(20,20);
  dl->AddCircleFilled(statusPos, 10, 
    consist_mode?ImColor(0.0f,1.0f,0.0f):ImColor(1.0f,0.0f,0.0f));
}

void Editor::editorDraw() {
  ImGui::Begin("Editor");
  ImGui::SetWindowFontScale(2.0);
  ImGui::Text("Viewport Resolution: %d*%d", viewportWH[0], viewportWH[1]);
  /*
  ImGui::Text("Background Color");
  ImGui::ColorEdit3("", (float*)&bkgColor);

  ImGui::Text("Draw Mode");
  ImGui::RadioButton("Color", &drawMode, 0);
  ImGui::RadioButton("Position", &drawMode, 2);

  ImGui::RadioButton("Normal", &drawMode, 1); ImGui::SameLine(); 
  ImGui::Checkbox("n in camera", &cam_coord_norm); ImGui::SameLine();
  ImGui::Checkbox("normalized", &norm_normalized);
  
  ImGui::RadioButton("Depth ", &drawMode, 3); ImGui::SameLine();
  ImGui::Checkbox("d in camera", &cam_coord_depth);
  */

  if(ImGui::Button("World Rotate X")) {
    world_trans = glm::rotate(world_trans, glm::radians(90.0f), glm::vec3(1.0,0.0,0.0));
  }

  ImGui::Text("Translate");
  float winWidth = ImGui::GetWindowWidth();
  ImGui::SetNextItemWidth(winWidth - 50);
  ImGui::SliderFloat("tX", &model_trans.x, trans_range_min.x, trans_range_max.x);
  ImGui::SetNextItemWidth(winWidth - 50);
  ImGui::SliderFloat("tY", &model_trans.y, trans_range_min.y, trans_range_max.y);
  ImGui::SetNextItemWidth(winWidth - 50);
  ImGui::SliderFloat("tZ", &model_trans.z, trans_range_min.z, trans_range_max.z);
  ImGui::Text("Rotate");
  ImGui::SetNextItemWidth(winWidth - 50);
  ImGui::SliderAngle("rX", &model_rotate.x);
  ImGui::SetNextItemWidth(winWidth - 50);
  ImGui::SliderAngle("rY", &model_rotate.y);
  ImGui::SetNextItemWidth(winWidth - 50);
  ImGui::SliderAngle("rZ", &model_rotate.z);
  ImGui::Text("Scale");
  ImGui::SetNextItemWidth(winWidth - 50);
  ImGui::SliderFloat("s", &model_scale, 
    scale_range.x, scale_range.y, "ratio = %.3f", 
    ImGuiSliderFlags_::ImGuiSliderFlags_Logarithmic);
  ImGui::Text("Material");
  ImGui::Checkbox("Simple material", &simpleMaterial);
  if(simpleMaterial) {
    ImGui::ColorEdit3("Albedo color", (float*)&model_color);
    ImGui::SliderFloat("Roughness", &model_rough, 0.0, 1.0, "ratio = %.3f");
    ImGui::SliderFloat("Metallic", &model_metal, 0.0, 1.0, "ratio = %.3f");
  }
  ImGui::Text("Render options");
  ImGui::RadioButton("SH base", &pipeline, 0); ImGui::SameLine();
  ImGui::RadioButton("SG base", &pipeline, 1);

  ImGui::RadioButton("Render Result", &visualize_mode, 0); ImGui::SameLine();
  ImGui::RadioButton("Render Probe", &visualize_mode, 1);
  ImGui::Text("Shadow options");
  ImGui::RadioButton("Field", &shadowMode, 1); ImGui::SameLine();
  ImGui::RadioButton("Cast", &shadowMode, 2); ImGui::SameLine();
  ImGui::RadioButton("None", &shadowMode, 0); 
  if(pipeline == 1) {
    ImGui::SameLine();
    static bool use_ss = true;
    ImGui::Checkbox("Self shadow", &use_ss);
    if(use_ss != sg_use_self_shadow) {
      sg_use_self_shadow = use_ss;
      NGPConnector::getConnector()->sendSGUseSShadow();
    }
  }
  ImVec2 buttSz(400,50);
  if(ImGui::Button(consist_mode?"Stop render": "Render", buttSz)) 
    consist_mode = !consist_mode;
  ImGui::Text("Experiments");
  runCmp = ImGui::Button("blender compare");
  if(ImGui::Button("other methods compare")) {
    NGPConnector::getConnector()->sendMethodsCompareInfo();
  }
  if(ImGui::Button("process decomposition")) {
    NGPConnector::getConnector()->sendProcessDecompositeCmpCall();
  }
  if(ImGui::Button("increase save index")) {
    PathMan::getIns().updateSaveInfos(-1);
  }
  ImGui::Text("current index: %d", PathMan::getIns().currentSaveID);

  ImGui::InputInt("set index", &target_save_id, 20);
  ImGui::SameLine();
  if(ImGui::Button("set")) {
    PathMan::getIns().updateSaveInfos(target_save_id);
  }

  saveRes = ImGui::Button("save current image");
  
  ImGui::End();



  ImGui::Begin("Camera View");
  ImGui::SetWindowFontScale(2.0);
  ImGui::Text("Camera Resolution: %d*%d", camportWH[0], camportWH[1]);
  ImGui::Text("Camera Fov y: %.2f", cam_fov);
  //ImGui::SliderAngle("FOV_y", &cam_fov, 5, 175);
  /*
  ImGui::Text("Camera Pose");
  ShaderInfo& sInfo = Scene::GetCurrentShaderInfo();
  const auto& pose = sInfo.invV;
  char posebuf[100];
  snprintf(posebuf, 100, "%.3f %.3f %.3f %.3f", 
    pose[0][0], pose[1][0], pose[2][0], pose[3][0]);
  std::string pose1(posebuf);
  snprintf(posebuf, 100, "%.3f %.3f %.3f %.3f", 
    pose[0][1], pose[1][1], pose[2][1], pose[3][1]);
  std::string pose2(posebuf);
  snprintf(posebuf, 100, "%.3f %.3f %.3f %.3f", 
    pose[0][2], pose[1][2], pose[2][2], pose[3][2]);
  std::string pose3(posebuf);
  ImGui::Text(pose1.c_str());
  ImGui::Text(pose2.c_str());
  ImGui::Text(pose3.c_str());
  */

  ImGui::Text("Main Light Direction: %.3f, %.3f, %.3f", 
    main_dir_light.x, main_dir_light.y, main_dir_light.z);
  ImGui::Text("Model Position: %.3f, %.3f, %.3f",
    model_center.x, model_center.y, model_center.z);
  // char dir_buf[100];
  // snprintf(dir_buf, 100, "%.3f, %.3f, %.3f", 
  //   main_dir_light.x, main_dir_light.y, main_dir_light.z);
  // ImGui::Text(dir_buf);
  
  // float focal =  camportWH[1] / (2.0*glm::tan(cam_fov/2.0));
  // ImGui::InputFloat("Focal Length", &focal);
  // cam_fov = 2*glm::atan(camportWH[1] / (2.0*focal));

  if(subCamport_texID >= 0) {
    ImGui::Image(
      (void*)(intptr_t)subCamport_texID, 
      {camportWH[0], camportWH[1]}, {0,1}, {1,0});
  }
  
  ImGui::End();

  drawMainLightDirection();
}