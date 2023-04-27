#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "utility.hpp"
#include "shader.hpp"
#include "libnpy/npy.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace std;

// only support 8 bit depth with 1, 3 or 4 channel images
void ImageTexture::TextureFromFile(std::string path, bool gamma) {

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
  if (data) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    if (nrComponents == 1)
      channel = GL_RED;
    else if (nrComponents == 3)
      channel = GL_RGB;
    else if (nrComponents == 4)
      channel = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, channel, width, height, 0, channel, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    this->id = textureID;
    this->dataFormat = GL_UNSIGNED_BYTE;
  }
  else {
    cout << "Texture failed to load at path: " << path << endl;
  }
  
}

HDRITexture::HDRITexture(std::string path) {
  int width, height, nrComponents;
  float *data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
  if(data) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    if (nrComponents == 1)
      channel = GL_RED;
    else if (nrComponents == 3)
      channel = GL_RGB;
    else if (nrComponents == 4)
      channel = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    this->id = textureID;
    this->dataFormat = GL_FLOAT;
  }
  else {
    cout << "Texture failed to load at path: " << path << endl;
  }
}

// Data from .npy file
/*
TextureArray::TextureArray(std::string path) {
  std::vector<npy::ndarray_len_t> shape; // D,H,W
  std::vector<float> data;
  npy::LoadArrayFromNumpy(path, shape, data);

  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32F, shape[2], shape[1], shape[0]);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  for(int d = 0; d<shape[0]; d++) {
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, d, 
      shape[2], shape[1], 1, GL_R32F, GL_RED, data.data()+d*shape[0]*shape[1]);
  }
  this->id = textureID;
  this->dataFormat = GL_FLOAT;
}

NpyTexture::NpyTexture(std::string path) {
  std::vector<npy::ndarray_len_t> shape; // H,W
  std::vector<float> data;
  npy::LoadArrayFromNumpy(path, shape, data);

  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 
    shape[1], shape[0], 0, GL_RED, GL_FLOAT, data.data()); 

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  this->id = textureID;
  this->dataFormat = GL_FLOAT;
}
*/

void RenderTexture::createRenderTexture(RTType type, bool newGenFbo, bool genMipMap) {
  if(newGenFbo) {
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);
  }

  // texture buffer for color buffer (allow sample)
  glGenTextures(1, &textureColorbuffer);
  if(!isCubeMap) {
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, sizeW, sizeH, 0, channel, dataFormat, NULL);
    // GL_LINEAR_MIPMAP_LINEAR: trilinear interpolation
    if(genMipMap)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    else
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if(genMipMap)
      glGenerateMipmap(GL_TEXTURE_2D);
  }
  else {
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureColorbuffer);
    for(unsigned int i = 0; i<6; i++) {
      glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 
        0, internalFormat, sizeW, sizeH, 0, 
        channel, dataFormat, NULL
      );
    }
    if(genMipMap) 
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    else
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    if(genMipMap)
      glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  }

  if(type == RTType::DEPTH_ONLY) {
    /*
      Difference between 
          glFramebufferTexture2D and glFramebufferTexture and glFramebufferRenderbuffer
        if use glFramebufferTexture2D, we must point out which texture type we attach,
        just like GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP_POSITIVE_X (one face of cubemap)
        they must be 2D

        if use glFramebufferTexture, we can render six faces of a cubemap in one pass
        but need to use GL_layer in geom shader

        if use glFramebufferRenderbuffer, the texture type must be GL_RENDERBUFFER
    */
    if(!isCubeMap)
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepthbuffer, 0);
    else
      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureDepthbuffer, 0);
    // while there is no color buffer, we should tell GL do not read or write color buffer explictly
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  }
  else {  
    if(!isCubeMap)
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachIdx, GL_TEXTURE_2D, textureColorbuffer, 0);
    else 
      glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachIdx, textureColorbuffer, 0);
  } 

  if(type == RTType::NORMAL) {
    // render buffer for depth and stencil buffer (do not sample)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, sizeW, sizeH);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
  }
  else if(type == RTType::NORMAL_NO_STENCIL) {
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, sizeW, sizeH);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
  }

  if(newGenFbo) {
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    // NOTE: must unbind, otherwise all render result will be written to this
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

RenderTexture::RenderTexture(int sizeW, int sizeH, RTType rtType, 
  GLint internalFormat, GLenum format, GLenum type, int attachIdx, 
  bool cubeMap, bool genMipMap): 
  rtType(rtType), attachIdx(attachIdx), 
  internalFormat(internalFormat), sizeW(sizeW), sizeH(sizeH) {

  this->dataFormat = type;
  this->channel = format;
  this->isCubeMap = cubeMap;

  createRenderTexture(rtType, true, genMipMap);
}

RenderTexture::RenderTexture(int sizeW, int sizeH, int fbo, RTType rtType, 
  GLint internalFormat, GLenum format, GLenum type, int attachIdx, 
  bool cubeMap, bool genMipMap): 
  rtType(rtType), attachIdx(attachIdx), 
  internalFormat(internalFormat), sizeW(sizeW), sizeH(sizeH) {

  this->id = fbo;
  this->dataFormat = type;
  this->channel = format;
  this->isCubeMap = cubeMap;


  createRenderTexture(rtType, false, genMipMap);
}

void RenderTexture::resize(int sizeW, int sizeH) {
  this->sizeH = sizeH;
  this->sizeW = sizeW;
  glDeleteFramebuffers(1, &id);
  createRenderTexture(rtType);
}

void RenderTexture::resize(int sizeW, int sizeH, int fbo) {
  this->sizeH = sizeH;
  this->sizeW = sizeW;
  this->id = fbo;
  createRenderTexture(rtType, false);
}

RenderTexture* RenderTexture::GenCubeMapFromTex2D(
  int sizeWH, Texture* tex, GLint internalFormat, GLenum format, GLenum type) {

  RenderTexture* cubeMap = new RenderTexture(
    sizeWH, sizeWH, RTType::NORMAL_NO_STENCIL, 
    internalFormat, format, type, 0, true
  );

  static Shader* projss = nullptr;
  if(!projss) {
    projss = new Shader(
      "./shaders/sphere2cube.vs",
      "./shaders/sphere2cube.fs"
    );
  }
  projss->use();
  projss->setTex("Tex", tex, 0);
  RenderSixFaces(cubeMap, projss, {0,0,0});
  return cubeMap;
}

void RenderTexture::RenderSixFaces(
  RenderTexture* rt, Shader* ss, glm::vec3 position, float near, float far) {

  glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, near, far);
  glm::mat4 vps[6];
  GetSixFacesTrans(position, proj, vps);
  unsigned int cubeVAO = GetStdCubeVAO();

  glBindFramebuffer(GL_FRAMEBUFFER, rt->id);
  glViewport(0, 0, rt->sizeW, rt->sizeH);
  
  ss->use();
  for(int i = 0; i<6; i++) {
    ss->setMat4("VP", vps[i]);

    // this is to tell OpenGL that:
    // the ith face of cubemap is the ATTACHMENT0
    // than next render will be written to ATTACHMENT0(i.e. the ith face)
    // this is a way to render a cubemap (the geom shader with gl_Layer is another way)
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
      GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, rt->textureColorbuffer, 0
    );

    // DO NOT FORGET glClear !!!
    // though each face has different texture, but depth buffer are the same,
    // if not clear, only one face will show.
    glClear(GL_DEPTH_BUFFER_BIT);
    /*RENDER CUBE*/
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

MultiRenderTexture::MultiRenderTexture(int sizeW, int sizeH):
  sizeW(sizeW), sizeH(sizeH) {
  glGenFramebuffers(1, &id);
}

void MultiRenderTexture::resize(int sizeW, int sizeH) {
  this->sizeH = sizeH;
  this->sizeW = sizeW;
  for(RenderTexture* rt : rts) {
    rt->resize(sizeW, sizeH, this->id);
  }
}

void MultiRenderTexture::addTarget(
    RenderTexture::RTType rtType,
    GLint internalFormat, 
    GLenum format, 
    GLenum type, 
    int attachIdx) {
  
  rts.push_back(
    new RenderTexture(
    sizeW, sizeH, id,
    rtType, internalFormat, 
    format, type, attachIdx)
  );
}

void MultiRenderTexture::addTarget(RenderTexture* rt) {
  rts.push_back(rt);
}

void MultiRenderTexture::completeAdd() {
  int buffersNum = rts.size();
  GLuint* attas = new GLuint[buffersNum];
  for(int i = 0; i<buffersNum; i++) 
    attas[i] = GL_COLOR_ATTACHMENT0 + i;
  glDrawBuffers(buffersNum, attas);
}

void MultiRenderTexture::addDepthOrStencil(bool needStencil) {
  if(needStencil) {
    // render buffer for depth and stencil buffer (do not sample)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, sizeW, sizeH);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
  }
  else {
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, sizeW, sizeH);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
  }
}

void MultiRenderTexture::checkAndCompleteAll() {
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// Only support RGB 8 bit images with 6 faces
CubeMapTexture::CubeMapTexture(vector<string> tex_faces) {
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  unsigned char *data;  
  for(unsigned int i = 0; i < 6; i++) {
    data = stbi_load(tex_faces[i].c_str(), &width, &height, &nrChannels, 0);
    if(data) {
      glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
        0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
      );
    }
    else {
      cout << "Cubemap texture failed to load at path: " << tex_faces[i] << endl;
    }
    stbi_image_free(data);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  this->id = textureID;
  this->dataFormat = GL_UNSIGNED_BYTE;
  this->channel = GL_RGB;
}