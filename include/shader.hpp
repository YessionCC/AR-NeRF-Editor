#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <functional>

#include "texture.hpp"

class Shader {
public:
  unsigned int ID = 0;
  std::function<void()> BeforeUse = nullptr;
  std::function<void()> OnClear = nullptr;

private:
  std::vector<GLenum> switches;

public:
  // constructor generates the shader on the fly
  // ------------------------------------------------------------------------
  Shader() {}
  Shader(
    const char* vertexPath, 
    const char* fragmentPath, 
    std::vector<unsigned int> switches = {GL_DEPTH_TEST},
    std::function<void()> beforeFunc = nullptr,
    std::function<void()> onClearFunc = nullptr,
    const char* geometryPath = nullptr
  );

  // if has texture, use this
  static const Shader* GetDefaultShader() {
    static const Shader* defaultShader = new Shader(
      "./shaders/default.vs", 
      "./shaders/default.fs"
    );
    return defaultShader;
  }
  //if no texture, use this
  static const Shader* GetPureShader() {
    static const Shader* pureShader = new Shader(
      "./shaders/pure.vs", 
      "./shaders/pure.fs"
    );
    return pureShader;
  }

  static const Shader* GetNormalShader() {
    static const Shader* normShader = new Shader(
      "./shaders/default.vs", 
      "./shaders/normal_w_p.fs"
    );
    return normShader;
  }

  static const Shader* GetNGP1Shader() {
    static const Shader* ngp1Shader = new Shader(
      "./shaders/default.vs", 
      "./shaders/ngp1.fs"
    );
    return ngp1Shader;
  }

  static const Shader* GetNGP2Shader() {
    static const Shader* ngp2Shader = new Shader(
      "./shaders/default.vs", 
      "./shaders/ngp2.fs"
    );
    return ngp2Shader;
  }

  static const Shader* GetPositionShader() {
    static const Shader* posShader = new Shader(
      "./shaders/default.vs", 
      "./shaders/position_w_p.fs"
    );
    return posShader;
  }

  static const Shader* GetDepthShader() {
    static const Shader* depthShader = new Shader(
      "./shaders/default.vs", 
      "./shaders/depth_p.fs"
    );
    return depthShader;
  }

  static const Shader* GetSDepthShader() {
    static const Shader* depthShader = new Shader(
      "./shaders/sdepth.vs", 
      "./shaders/sdepth.fs"
    );
    return depthShader;
  }

  static const Shader* GetPCShader() {
    static const Shader* pcShader = new Shader(
      "./shaders/pc.vs", 
      "./shaders/pc.fs"
    );
    return pcShader;
  }

  static const Shader* GetSGShader() {
    static const Shader* sgShader = new Shader(
      "./shaders/sg.vs", 
      "./shaders/sg.fs"
    );
    return sgShader;
  }


  inline const std::vector<GLenum>& GetSwitches() const {return switches;}

  // activate the shader
  // You'd better first call use, than set value to avoid bugs !!
  inline void use() const { 
    for(auto sw: switches) {
      glEnable(sw);
    }
    if(BeforeUse)
      BeforeUse();
    glUseProgram(ID); 
  }

  inline void clear() const {
    for(auto sw: switches) {
      glDisable(sw);
    }
    if(OnClear)
      OnClear();
  }

  // utility uniform functions
  // ------------------------------------------------------------------------
  inline void setBool(const std::string &name, bool value) const{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
  }
  // ------------------------------------------------------------------------
  inline void setInt(const std::string &name, int value) const{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
  }
  inline void setUInt(const std::string &name, unsigned int value) const{ 
    glUniform1ui(glGetUniformLocation(ID, name.c_str()), value); 
  }
  // ------------------------------------------------------------------------
  inline void setFloat(const std::string &name, float value) const{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
  }
  // ------------------------------------------------------------------------
  inline void setVec2(const std::string &name, const glm::vec2 &value) const{ 
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
  }
  inline void setVec2(const std::string &name, float x, float y) const{ 
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
  }
  // ------------------------------------------------------------------------
  inline void setVec3(const std::string &name, const glm::vec3 &value) const{ 
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
  }
  inline void setVec3(const std::string &name, float x, float y, float z) const{ 
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
  }
  // ------------------------------------------------------------------------
  inline void setVec4(const std::string &name, const glm::vec4 &value) const{ 
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
  }
  inline void setVec4(const std::string &name, float x, float y, float z, float w) { 
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
  }
  // ------------------------------------------------------------------------
  inline void setMat2(const std::string &name, const glm::mat2 &mat) const{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }
  // ------------------------------------------------------------------------
  inline void setMat3(const std::string &name, const glm::mat3 &mat) const{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }
  // ------------------------------------------------------------------------
  inline void setMat4(const std::string &name, const glm::mat4 &mat) const{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }

  // do not use it to set rendertexture !!
  inline void setTex(std::string name, const Texture* tex, int index) const{
    glActiveTexture(GL_TEXTURE0 + index);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), index);
    glBindTexture(GL_TEXTURE_2D, tex->id);
  }
  // do not use it to set cube map!!!
  inline void setRenderTexColorBuff(std::string name, const RenderTexture* tex, int index) const{
    glActiveTexture(GL_TEXTURE0 + index);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), index);
    glBindTexture(GL_TEXTURE_2D, tex->textureColorbuffer);
  }

  inline void setCubeMap(std::string name, const CubeMapTexture* tex, int index) const {
    glActiveTexture(GL_TEXTURE0 + index);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), index);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex->id);
  }

  inline void setCubeMapRT(std::string name, const RenderTexture* tex, int index) const {
    glActiveTexture(GL_TEXTURE0 + index);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), index);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex->textureColorbuffer);
  }

private:
  // utility function for checking shader compilation/linking errors.
  // ------------------------------------------------------------------------
  void checkCompileErrors(GLuint shader, std::string type);
};