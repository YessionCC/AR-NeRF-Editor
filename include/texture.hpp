#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <glad/glad.h> 
#include <glm/glm.hpp>

class Shader;
class Texture {
public:
  // Notice: For renderTex, its id is the framebuffer id, not texture id!!!
  // must use RenderTexture.textureColorbuffer
  unsigned int id = 0;
  GLenum channel;
  GLenum dataFormat;

public:
  virtual ~Texture() {}

  
};

class ImageTexture: public Texture {
public:
  std::string type; // diff / spec / ...
  std::string path; // used for mapping the same texture

  ImageTexture(std::string path, std::string type, bool gamma = false): 
    type(type), path(path) {
    this->dataFormat = GL_UNSIGNED_BYTE;
    TextureFromFile(path, gamma);
  }

private:
  void TextureFromFile(std::string path, bool gamma);
};

class HDRITexture: public Texture {
public:
  HDRITexture(std::string path);
};

/*
// from D,H,W npy data (float32)
class TextureArray: public Texture {
public:
  TextureArray(std::string path);
};

// from H,W npy data (float32)
class NpyTexture: public Texture {
public:
  NpyTexture(std::string path);
};
*/

// NOTICE: how to use renderTexture correctly
// 1. data type shhould be matched
// 2. glClear should be called to clear old data
// 3. glViewport should be called if the texture size is not equal to screen
class RenderTexture: public Texture {
public:
  enum RTType{
    // color (texture buffer, allow sample), depth and stencil(render buffer, no sample)
    NORMAL, 
    // no stencil buffer, the rest as above
    NORMAL_NO_STENCIL,
    // only color buffer(allow sample)
    COLOR_ONLY,
    // only depth buffer(allow sample)
    DEPTH_ONLY
  };

public:
  RTType rtType;

  union {
    unsigned int textureColorbuffer;
    unsigned int textureDepthbuffer;
  };
  unsigned int rbo; // depth and stencil buffer
  
  int attachIdx;
  bool isCubeMap = false;
  GLenum internalFormat;

  // if the size do not match the screen size, 
  // need to use glviewport before render
  int sizeW, sizeH;

public:
  RenderTexture(int sizeW, int sizeH, 
    RTType rtType = NORMAL,
    GLint internalFormat=GL_RGB, 
    GLenum format=GL_RGB, 
    GLenum type=GL_UNSIGNED_BYTE, 
    int attachIdx = 0, 
    bool cubeMap = false, 
    bool genMipMap = false);

  // use this constructor for multi-output, the its id is invalid
  RenderTexture(
    int sizeW, int sizeH, int fbo,
    RTType rtType = NORMAL,
    GLint internalFormat=GL_RGB, 
    GLenum format=GL_RGB, 
    GLenum type=GL_UNSIGNED_BYTE, 
    int attachIdx = 0,
    bool cubeMap = false,
    bool genMipMap = false);

  // if after resize, the framebuffer's size do not as same as the screen
  // glViewport should be called later
  void resize(int sizeW, int sizeH);

  void resize(int sizeW, int sizeH, int fbo);

  // tex should be ImageTex or HDRITex
  // return a color cubemap texture
  // because we always use a cube to transform, 
  // so the size of rt also square 
  static RenderTexture* GenCubeMapFromTex2D(
    int sizeWH, Texture* tex,
    GLint internalFormat=GL_RGB, 
    GLenum format=GL_RGB, 
    GLenum type=GL_UNSIGNED_BYTE);

  // rt should be a cube rendertexture
  static void RenderSixFaces(
    RenderTexture* rt, Shader* ss, glm::vec3 position,
    float near = 0.01f, float far = 100.0f);

private:
  void createRenderTexture(RTType type, bool newGenFbo = true, bool genMipMap = false);
};

// only its id is invalid, dataType should be get from its sub-texture
// add order:
// startAdd->addTargets(do not interpret by others)->
// completeAdd->addDepthOrStencil->completeAll
class MultiRenderTexture: public Texture {
public:
  std::vector<RenderTexture*> rts;
  int sizeW, sizeH;
  GLuint rbo;

  MultiRenderTexture(int sizeW, int sizeH);

  void resize(int sizeW, int sizeH);

  void startAdd() {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
  }

  // no more than two normal rt should be added
  void addTarget(
    RenderTexture::RTType rtType = RenderTexture::RTType::NORMAL,
    GLint internalFormat=GL_RGB, 
    GLenum format=GL_RGB, 
    GLenum type=GL_UNSIGNED_BYTE, 
    int attachIdx = 0);

  void addTarget(RenderTexture* rt);

  // after calling it, do not add targets, but add depth or stencil buffer
  void completeAdd();
  void addDepthOrStencil(bool needStencil = false);
  void checkAndCompleteAll();
};


class CubeMapTexture: public Texture {

public:
  CubeMapTexture(std::vector<std::string> tex_faces);
};
