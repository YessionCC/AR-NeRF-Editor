#include "specialModel.hpp"

#include "scene.hpp"
#include "texture.hpp"
#include "utility.hpp"

using namespace std;

void SpecialModel::draw(const Shader* s) {
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, vtxNumToDraw);
  glBindVertexArray(0);
}


Grid::Grid(float scale): SpecialModel(
  new Shader(
    "./shaders/grid.vs",
    "./shaders/grid.fs", 
    {GL_DEPTH_TEST, GL_BLEND},
    [](){glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);}), 
    TRANSPARENT_PRIO, GetClipSpaceVAO(), 6),
    scale(scale){
      this->mustUseSelfShader = true;
}


Skybox::Skybox(std::vector<std::string> tex_face_paths): SpecialModel(
  new Shader(
    "./shaders/skybox.vs",
    "./shaders/skybox.fs", 
    {GL_DEPTH_TEST}, 
    [](){glDepthFunc(GL_LEQUAL);},  
    // we set skybox depth = 1.0, when opengl clear depth buffer, 
    // its default fill value is 1.0, so if we not set LESS AND EQUAL,
    // there will be nothing to present
    [](){glDepthFunc(GL_LESS);}
  ), TRANSPARENT_PRIO, GetStdCubeVAO(), 36), cubemap(tex_face_paths) {
  this->mustUseSelfShader = true;
}

void Skybox::beforeDraw() {
  shader->setCubeMap("skybox", &cubemap, 0);
  if(cpRT) {
    shader->setCubeMapRT("cpTex", cpRT, 1);
  }

  ShaderInfo& sif = Scene::GetCurrentShaderInfo();
  glm::mat4 V = sif.V;
  V = glm::mat4(glm::mat3(V));
  shader->setMat4("VrotP", sif.P*V);
}


Plane::Plane(const Shader* s, unsigned int prio): 
  SpecialModel(s, prio, GetClipSpaceVAO(), 6) {}