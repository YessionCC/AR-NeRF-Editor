#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.hpp"

class Scene;
class Postprocess;
class Editor;
class ModelBase;
class Model;
class NGPConnector;
struct SavePack;

class Viewport {
private:
  int windowHeight = 720;
  int windowWidth = 1024;
  ViewportCamera vpCam;

  //Postprocess* pp;
  GLFWwindow* window;

  NGPConnector* conn = nullptr;

public:
  float scrollSensitivity = 2.0f;
  float translateSensitivity = 0.1f;
  float rotateSensitivity = 0.4f;

private:
  unsigned int currentSelectObjID = 0;
  float deltaTime = 0.0f;

private:
  // input state
  double mouseX = 0, mouseY = 0;
  bool mouseMidleButtPress = false;
  bool mouseRightButtPress = false;
  bool mouseLeftButtPress = false;

private:
  Viewport() {
    vpCam.wDivH = 1.0f*windowWidth/windowHeight;
  }
  void calcDeltaTime();

public:
  Viewport(const Viewport& rhs) = delete;
  Viewport& operator=(const Viewport& rhs) = delete;

  void resizeWindow(int windowHeight, int windowWidth);

  void Init();
  void Start(Scene& scene);

  inline int GetHeight() const {return windowHeight;}
  inline int GetWidth() const {return windowWidth;}
  inline unsigned int GetSelectID() const {return currentSelectObjID;}
  inline float GetDeltaTime() const {return deltaTime;}

  static inline Viewport& GetInstance() {
    static Viewport instance;
    return instance;
  }

  static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
  static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
  static void file_drop_in(GLFWwindow* window, int cnt, const char** paths);

private:
  void show_preview_in_editor(Editor& editor);
  // SH Process
  void SH_PROCESS_visual_probe(Editor& editor, std::vector<ModelBase *> &models);
  void SH_PROCESS_draw_contact(Editor& editor, std::vector<ModelBase *> &models, 
    SavePack* sp = nullptr);
  void SH_Save_contact(Editor& editor, std::vector<ModelBase *> &modelsALL, 
    std::vector<Model *> &modelsMesh);
  void SG_PROCESS_draw_contact(Editor& editor, std::vector<ModelBase *> &models, 
    bool send_save_flag = false);
};