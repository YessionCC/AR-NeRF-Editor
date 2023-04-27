#pragma once

#include "texture.hpp"
#include "utility.hpp"
#include "shader.hpp"
#include <functional>

class Postprocess {
public:
  // param1: input, param2: output, param3: auxiliary
  std::function<void(
    RenderTexture*, 
    RenderTexture*, 
    RenderTexture*)> 
    processFunc = nullptr;

private:
  RenderTexture* texMain;
  RenderTexture* texAux1;
  RenderTexture* texAux2;

  unsigned int clipVAO;
  Shader* clipShader;

public:
  Postprocess(int winWidth, int winHeight): 
    texMain(new RenderTexture(winWidth, winHeight)),
    texAux1(new RenderTexture(winWidth, winHeight, RenderTexture::RTType::COLOR_ONLY)),
    texAux2(new RenderTexture(winWidth, winHeight, RenderTexture::RTType::COLOR_ONLY)),
    clipVAO(GetClipSpaceVAO()),
    clipShader(new Shader(
      "./shaders/clip.vs",
      "./shaders/clip.fs"
    )) {

  }

  void resetSize(int width, int height) {
    texMain->resize(width, height);
    texAux1->resize(width, height);
    texAux2->resize(width, height);
  }

  inline RenderTexture* getPostprocessRT() {
    return texMain;
  }

  void renderOutputToPostprocess() const {
    glBindFramebuffer(GL_FRAMEBUFFER, texMain->id);
  }
  void stopRenderOutputToPostprocess() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void renderToScreen() const {
    clipShader->setRenderTexColorBuff("texMain", texMain, 0);
    clipShader->setRenderTexColorBuff("texAux1", texAux1, 1);
    clipShader->setRenderTexColorBuff("texAux2", texAux2, 2);
    glActiveTexture(GL_TEXTURE0);

    clipShader->use();

    // clear color buffer is not neccessary
    // but NOTICE: must clear depth color or disable depth test
    // otherwise, after first render, the depth buffer will be all set to 0.0
    // with GL_LESS, next will not pass depth test
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(clipVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    clipShader->clear();
  }

  // for debug
  void showRenderTexture(RenderTexture* rt) const {
    clipShader->setRenderTexColorBuff("texMain", rt, 0);
    clipShader->use();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(clipVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    clipShader->clear();
  }

  void process() {
    if(processFunc) {
      processFunc(texMain, texAux1, texAux2);
    }
  }
};