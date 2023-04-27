#include "viewport.hpp"

#include <iostream>
#include "model.hpp"
#include "texture.hpp"
#include "postprocess.hpp"
#include "scene.hpp"

#include "editor.hpp"
#include "serializer.hpp"
#include "insert_utils.hpp"
#include "connector.hpp"
#include "pathManager.hpp"
#include "blender_cmp.hpp"

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb/stb_image.h"

using namespace std;

void Viewport::framebuffer_size_callback(GLFWwindow* window, int width, int height) {

  Viewport& cvp = GetInstance();
  cvp.resizeWindow(height, width);
  glViewport(0, 0, width, height);
}

void Viewport::mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
  static bool firstMouse = true;
  Viewport& cvp = GetInstance();

  if(firstMouse) {
    cvp.mouseX = xposIn;
    cvp.mouseY = yposIn;
    firstMouse = false;
  }
  float xoffset = xposIn - cvp.mouseX;
  float yoffset = cvp.mouseY - yposIn; // reversed y-coordinates

  cvp.mouseX = xposIn;
  cvp.mouseY = yposIn;

  if(cvp.mouseRightButtPress) {
    float scale = 0.03*cvp.rotateSensitivity;
    cvp.vpCam.rotateAroundFocus(xoffset*scale, yoffset*scale);
  }
  if(cvp.mouseMidleButtPress) {
    float scale = 0.03*cvp.translateSensitivity;
    cvp.vpCam.translateInXYPlane(-xoffset*scale, -yoffset*scale);
  }
}

void Viewport::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  Viewport& cvp = GetInstance();
  cvp.vpCam.translateAlongZ(-yoffset*cvp.scrollSensitivity*0.03);
}

void Viewport::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  Viewport& cvp = GetInstance();
  static double pressX, pressY, pressTime;
	if (action == GLFW_PRESS || action == GLFW_RELEASE) {
    if(action == GLFW_PRESS) {
      glfwGetCursorPos(window, &pressX, &pressY);
      pressTime = glfwGetTime();
    }
    else {
      double releaseX, releaseY, releaseTime;
      glfwGetCursorPos(window, &releaseX, &releaseY);
      releaseTime = glfwGetTime();
      if(releaseTime-pressTime < 0.2 || (pressX==releaseX && pressY==releaseY)) {
        // now cvp.mouseLeftButtPress ... are still true, we can use it to distinguish
        // which mouse click
      }
    }
    switch(button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      cvp.mouseLeftButtPress = action;
      break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
      cvp.mouseMidleButtPress = action;
      break;
    case GLFW_MOUSE_BUTTON_RIGHT:
      cvp.mouseRightButtPress = action;
      break;
    default:
      return;
    }
  }
}

void Viewport::file_drop_in(GLFWwindow* window, int cnt, const char** paths) {
  if(cnt > 1) {
    std::cout << "Not support files > 1" <<std::endl;
    return;
  }
  std::string path(paths[0]);
  size_t dot_idx = path.find_last_of(".");
  if(dot_idx == -1) {
    std::cout << "Invalid file" <<std::endl;
    return;
  }
  std::string type = path.substr(dot_idx+1);
  if(type != "obj") {
    std::cout << "Only support substitude obj model" <<std::endl;
    return;
  }
  Scene* scene = Scene::CurrentScene();
  Editor& editor = Editor::getInstance();
  std::vector<Model*> ms; std::vector<PointCloud*> pcs;
  scene->getModelsByType(ms, ModelType::Meshes);
  scene->getModelsByType(pcs, ModelType::Points);
  if(pcs.empty()) {
    std::cout << "Something Wrong" << path <<std::endl;
    return;
  }
  editor.currentModelPath = path;
  if(ms.empty()) {
    std::cout << "Add model, path: " << path <<std::endl;
    Model* _newModel = new Model(path);
    scene->addModel(_newModel);
    editor.initRange(pcs[0], _newModel);
  }
  else {
    std::cout << "Substitude model, path: " << path <<std::endl;
    Model* _newModel = new Model(path);
    editor.updateModelRange(ms[0], _newModel);
    *(ms[0]) = *_newModel; // here will cause memory leak

    NGPConnector::getConnector()->sendSForSSDFInfo(path);
  }
}

void Viewport::resizeWindow(int windowHeight, int windowWidth) {
  this->windowHeight = windowHeight;
  this->windowWidth = windowWidth;
  this->vpCam.wDivH = 1.0f*windowWidth/windowHeight;
  //pp->resetSize(windowWidth, windowHeight);
  int* wh = Editor::getInstance().viewportWH;
  wh[0] = windowWidth; wh[1] = windowHeight;
}

void Viewport::calcDeltaTime() {
  float currentFrame = static_cast<float>(glfwGetTime());
  static float lastFrame = currentFrame;
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;
}

void Viewport::Init() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(windowWidth, windowHeight, "Viewer", NULL, NULL);
  
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }
  
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetDropCallback(window, file_drop_in);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
  }
  Editor::getInstance().init(window, windowWidth, windowHeight);
}

void Viewport::SH_PROCESS_visual_probe(Editor& editor, std::vector<ModelBase *> &models) {
  conn->sendMapsSimple(vpCam, models, editor.camportWH[0], editor.camportWH[1]); // just for syn
  conn->sendProbePos(models);
  if(editor.shadowMode == 2)
    conn->recvMainDirectionLight();
  //conn->recvRenderCompleteFlag();
}

void Viewport::SH_PROCESS_draw_contact(Editor& editor, 
  std::vector<ModelBase *> &models, SavePack* sp) {
  if(editor.simpleMaterial) {
    conn->sendMapsSimple(vpCam, models, editor.camportWH[0], editor.camportWH[1]);
    conn->sendMaterial();
  }
  else
    conn->sendMapsComplex(vpCam, models, editor.camportWH[0], editor.camportWH[1]);
  //conn->sendShadowInfo(vpCam, editor.camportWH[1], editor.camportWH[0]);
  conn->sendProbePos(models);
  if(editor.shadowMode == 2)
    conn->recvMainDirectionLight();
  conn->sendCamPose();
  if(sp) {
    conn->sendRenderSaveCall(sp);
  }
  else {
    conn->sendRenderCall();
  }
  //SaveInsertInfos(vpCam, models, editor.camportWH[0], editor.camportWH[1], "./meta");
  glViewport(0,0,windowWidth, windowHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  conn->recvRenderCompleteFlag();
}

void Viewport::SG_PROCESS_draw_contact(Editor& editor, 
  std::vector<ModelBase *> &models, bool send_save_flag) {

  conn->sendProbePos(models);
  conn->recvSGLights();
  conn->sendMapsSimple(vpCam, models, editor.camportWH[0], editor.camportWH[1], true);
  conn->sendCamPose();

  if(editor.shadowMode == 2)
    conn->recvMainDirectionLight();
  
  if(send_save_flag) {
    //conn->sendRenderSaveCall();
  }
  else {
    conn->sendRenderCall();
  }

  glViewport(0,0,windowWidth, windowHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  conn->recvRenderCompleteFlag();
}

void Viewport::SH_Save_contact(
  Editor& editor, std::vector<ModelBase *> &modelsALL, 
  std::vector<Model *> &modelsMesh) {
  editor.runCmp = false;
  // save insert result
  SavePack sp{"nerf", true};
  SH_PROCESS_draw_contact(editor, modelsALL, &sp);
  // save non-object image
  sp = {"env", false};
  for(auto m: modelsMesh) {
    m->transform(editor.model_trans+glm::vec3(1000,0,0), 
    editor.model_rotate, glm::vec3(editor.model_scale));
  }
  int sd = editor.shadowMode;
  editor.shadowMode = 0;
  SH_PROCESS_draw_contact(editor, modelsALL, &sp);
  editor.shadowMode = sd;
  
  if(!editor.saveRes) {
    BlenderCmp::getInstance().run_cmp();
  }
  editor.saveRes = false;
}

void Viewport::show_preview_in_editor(Editor& editor) {
  editor.subCamport_texID = 
    ShowCameraView(vpCam, nullptr, editor.camportWH[0], editor.camportWH[1]);
  glViewport(0,0,windowWidth, windowHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Viewport::Start(Scene& scene) {

  Camera::SetCurrentCamera(&vpCam);
  //pp = new Postprocess(windowWidth, windowHeight);

  Editor& editor = Editor::getInstance();
  conn = NGPConnector::getConnector();

  Serializer::loadAll(PathMan::getIns().ResultPath() + "viewer_config.ini");
  int prev_pipeline = editor.pipeline;
  conn->recvHWF();
  conn->recvBlenderTrans();
  glfwSetWindowSize(window, editor.viewportWH[0], editor.viewportWH[1]);

  while (!glfwWindowShouldClose(window)) {
    calcDeltaTime();

    glfwSetWindowTitle(window, 
      ("Viewer, fps: "+std::to_string(int(1.0/deltaTime)+2)+
        " time: "+std::to_string(deltaTime)).c_str());

    glViewport(0,0,windowWidth, windowHeight);
    
    glClearColor(
      editor.bkgColor.r, editor.bkgColor.g, editor.bkgColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    vpCam.fov = glm::degrees(editor.cam_fov);
    const Shader* renderShader = nullptr;

    auto& models = Scene::CurrentScene()->getAllModels();
    std::vector<Model*> ms;
    Scene::CurrentScene()->getModelsByType(ms, ModelType::Meshes);
    for(auto m: ms) {
      m->transform(editor.model_trans, 
      editor.model_rotate, glm::vec3(editor.model_scale));
    }

    if(editor.pipeline != prev_pipeline) {
      prev_pipeline = editor.pipeline;
      conn->sendSForSSDFInfo(editor.currentModelPath);
    }

    if(editor.pipeline == 0) { // SH pipeline
      if(editor.consist_mode) {
        if(editor.visualize_mode == 1) { // only draw probe
          SH_PROCESS_visual_probe(editor, models);
        }
        if(editor.visualize_mode == 0) { // intact SH pipeline
          SH_PROCESS_draw_contact(editor, models, nullptr);
        }
      }
      if(editor.runCmp || editor.saveRes) {
        SH_Save_contact(editor, models, ms);
      }
    }
    else if(editor.pipeline == 1) { // SG pipeline
      if(editor.consist_mode) {
        if(editor.visualize_mode == 1) { // only draw probe
          
        }
        if(editor.visualize_mode == 0) { // intact SH pipeline
          //SG_PROCESS_draw_contact(editor, models, false);
          SH_PROCESS_draw_contact(editor, models, nullptr);
        }
      }
      if(editor.runCmp || editor.saveRes) {
        SH_Save_contact(editor, models, ms);
      }
    }

    
    
    show_preview_in_editor(editor);
    Scene::CurrentScene()->renderAll(renderShader);
    editor.update();

    // ~@DEBUG!!!!: this is for debug texture2D
    //pp->showRenderTexture(ssr.output_rt);
    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  
  Serializer::dumpAll(PathMan::getIns().ResultPath() + "viewer_config.ini");
  editor.terminate();
  glfwDestroyWindow(window);
  glfwTerminate();
}


