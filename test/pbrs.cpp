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

int main() {
  Viewport::GetInstance().Init();

  NGPConnector conn("127.0.0.1", 5001);
  NGPConnector::setConnector(&conn);

  Scene scene;
  Scene::UseScene(&scene);

  ImageTexture* cerb_A = new ImageTexture(
    "./models/cerberus/Texture/Cerberus_A.tga","Albedo"
  );
  ImageTexture* cerb_AO = new ImageTexture(
    "./models/cerberus/Texture/Cerberus_AO.tga","AO"
  );
  ImageTexture* cerb_M = new ImageTexture(
    "./models/cerberus/Texture/Cerberus_M.tga","Metallic"
  );
  ImageTexture* cerb_N = new ImageTexture(
    "./models/cerberus/Texture/Cerberus_N.tga","Normal"
  );
  ImageTexture* cerb_R = new ImageTexture(
    "./models/cerberus/Texture/Cerberus_R.tga","Rough"
  );
  
  Model model("./models/cerberus/Cerberus_LP.FBX");
  NGPConnector::getConnector()->sendShadowFieldInfo("./models/cerberus/Cerberus_LP.FBX");
  PointCloud pc("./pc.ply");

  model.BeforeDraw = [&](Model* model){
    model->curShader->setTex("texture_albedo", cerb_A, 0);
    model->curShader->setTex("texture_metal", cerb_M, 1);
    model->curShader->setTex("texture_rough", cerb_R, 2);
    model->curShader->setTex("texture_ao", cerb_AO, 3);
    //model->shader->setTex("AO", cerb_AO, 6);
    //model->shader->setTex("NormalMap", cerb_N, 7);
  };

  Editor::getInstance().initRange(&pc, &model);

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