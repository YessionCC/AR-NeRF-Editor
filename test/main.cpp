#include "viewport.hpp"
#include "specialModel.hpp"

#include "shader.hpp"
#include "light.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "model.hpp"
#include "connector.hpp"
#include "editor.hpp"
#include "sg.hpp"
#include "pathManager.hpp"

int main(int argc, char** argv) {
  std::string curScene = "scene0";
  if(argc == 1) {
    printf("Not set scene, use default scene!\n");
  }
  else {
    curScene = argv[1];
  }
  PathMan::getIns().SetCurScene(curScene);

  Viewport::GetInstance().Init();

  NGPConnector conn("127.0.0.1", 5001);
  NGPConnector::setConnector(&conn);

  Scene scene;
  Scene::UseScene(&scene);

  Editor& edt = Editor::getInstance();

  //Model model("./models/sphere.obj", Shader::GetSGShader());
  PointCloud pc(PathMan::getIns().ScenePath() + "pc.ply");

  std::string mpath = "./models/dragon.obj";
  Model model(mpath);
  NGPConnector::getConnector()->sendSForSSDFInfo(mpath);
  edt.currentModelPath = mpath;
  edt.initRange(&pc, &model);

  
  model.BeforeDraw = [&](Model* model){
    Editor& editor = Editor::getInstance();
    model->curShader->setBool("use_svBRDF", false);
    model->curShader->setVec3("_albedo", editor.model_color);
    model->curShader->setFloat("_metallic", editor.model_metal);
    model->curShader->setFloat("_rough", editor.model_rough);
    for(int i = 0; i<MAX_SG_NUM; i++) {
      editor.lsgs[i].setToShader(model->curShader, "lSGs", i);
    }
    //model->shader->setTex("AO", cerb_AO, 6);
    //model->shader->setTex("NormalMap", cerb_N, 7);
  };
  

  scene.addModel(&model);
  scene.addModel(&pc);

  //scene.addModel(&plane);
  //scene.skybox = &skybox;

  //DirectionalLight lt({0,15,0}, {0,-15,0});
  PointLight lt({0,18,0});
  scene.light = &lt;

  scene.Init();

  Viewport::GetInstance().Start(scene);

}