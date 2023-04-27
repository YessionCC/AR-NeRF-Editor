#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <functional>

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"
#include "shader.hpp"
#include "utility.hpp"

class AssimpData {
public:
  // model data 

  // if no divide mesh, this value keep -1, otherwise is the index of the mesh in model
  int subMeshID = -1;
  std::vector<Mesh>    meshes;
  std::string directory;
  std::string modelName;
  bool gammaCorrection;

  AssimpData() {}
  // constructor, expects a filepath to a 3D model.
  AssimpData(std::string const &path, bool gamma = false) : gammaCorrection(gamma) {
    loadModel(path);
  }

  void getBound(glm::vec3& boundMin, glm::vec3& boundMax) const;
    
private:
  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes std::vector.
  void loadModel(std::string const &path);

  // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
  void processNode(aiNode *node, const aiScene *scene);

  Mesh processMesh(aiMesh *mesh, const aiScene *scene);

  // checks all material textures of a given type and loads the textures if they're not loaded yet.
  // the required info is returned as a Texture struct.
  std::vector<ImageTexture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};

class PlyData {
private:
  struct PosCol{
    glm::vec3 point;
    glm::vec3 color;
  };

public:
  unsigned int VAO;
  int pointsNum;
  std::vector<PosCol> pcData;
  glm::vec3 minBound;
  glm::vec3 maxBound;

private:
  unsigned int VBO;

public:
  PlyData() {}
  PlyData(std::string path) {loadPly(path);}

  void upload();
  void draw();

private:
  void loadPly(std::string path);
};

enum ModelType {
  Any,
  Unknown,
  Meshes,
  Points,
  Special
};

class ModelBase {
public:
  ModelType mType = ModelType::Unknown;
protected:
  glm::mat4 transformMat;
public:
  // the id in the scene, after init scene, the value is valid
  unsigned int ID_scene = -1; 
  const Shader* shader;
  const Shader* curShader; // currently used shader to draw it
  // if customDraw not null, render will call it instead beforeDraw() and draw()
  // the param: shader, is the shader prepared for normal draw()
  // and the shader is in using state
  std::function<void(ModelBase* model, const Shader*)> customDraw = nullptr;

  // the larger, the later it will be rendered
  unsigned int renderPriority;
  
  // if set true, when render with other shader, this model will be skipped
  bool mustUseSelfShader = false;

  // if true, the model will gen shadow
  bool generateShadow = false;
  // if true, shader will accept ShadowMap params
  bool receiveShadow = false;
  // any render in defer queue should have same shader
  bool joinDeferRender = false;

public:
  virtual ~ModelBase() {}
  ModelBase(){}
  ModelBase(const Shader* shader, unsigned int renderPrio): 
    transformMat(1.0f), shader(shader?shader:Shader::GetDefaultShader()),
    renderPriority(renderPrio) {}

  // used for set some value to shader
  virtual void beforeDraw() {}
  virtual void draw(const Shader* s = nullptr) = 0;

  inline const glm::mat4& GetTransformMat() const {return transformMat;}
  inline void SetTransformMat(const glm::mat4 mat) {transformMat=mat;}
  
  inline void translate(glm::vec3 trans) {
    transformMat = glm::translate(transformMat, trans);
  }
  inline void scale(glm::vec3 scale) {
    transformMat = glm::scale(transformMat, scale);
  }
  // rot in deg, by x-y-z
  inline void rotate(glm::vec3 rot) {
    transformMat = glm::rotate(transformMat, glm::radians(rot.x), {1.0f, 0.0f, 0.0f});
    transformMat = glm::rotate(transformMat, glm::radians(rot.y), {0.0f, 1.0f, 0.0f});
    transformMat = glm::rotate(transformMat, glm::radians(rot.z), {0.0f, 0.0f, 1.0f});
  }

  inline void transform(glm::vec3 trans, glm::vec3 rot, glm::vec3 scl) {
    glm::mat4 s = glm::scale(glm::mat4(1), scl);
    glm::mat4 r = GetRotationMatrix(rot);
    glm::mat4 t = glm::translate(glm::mat4(1), trans);
    transformMat = t*r*s;
  }

  // to make the same shader closer
  virtual bool operator<(const ModelBase& rhs) const {
    return renderPriority<rhs.renderPriority || 
      (renderPriority==rhs.renderPriority && shader < rhs.shader);
  }
};


class Model: public ModelBase {
private:
  // we allow shallow copy to avoid large amount of data copy
  // also, we allow same model with different transform, shader, etc...
  AssimpData* data = nullptr; 

public:
  std::function<void(Model* model)> BeforeDraw = nullptr;

public:
  Model() {mType = ModelType::Meshes;}
  Model(std::string const &path, const Shader* shader = nullptr, 
    unsigned int renderPrio = SOLID_PRIO, bool gamma = false): 
    ModelBase(shader, renderPrio), data(new AssimpData(path, gamma)){
    mType = ModelType::Meshes;
  }

  void beforeDraw() override {
    if(BeforeDraw) BeforeDraw(this);
  }
  
  // draws the model, and thus all its meshes
  // if set shader, the model will not use the shader set when construct
  void draw(const Shader* s = nullptr) {
    s = s?s:shader;
    for(unsigned int i = 0; i < data->meshes.size(); i++)
      data->meshes[i].Draw(*s);
  }

  // after call it, the raw model is invalid
  void separateInMesh(std::vector<Model>& models) const;

  inline int GetSubMeshID() const {
    return data->subMeshID;
  }
  inline std::string GetModelName() const {
    return data->modelName;
  }

  glm::vec3 getCenter() const {
    glm::vec3 boundMin, boundMax;
    data->getBound(boundMin, boundMax);
    return 0.5f*(boundMin+boundMax);
  }

  glm::vec3 getCenter_W() const {
    return transformMat*glm::vec4(getCenter(), 1.0f);
  }

  float getRadius() const {
    glm::vec3 boundMin, boundMax;
    data->getBound(boundMin, boundMax);
    glm::vec3 box = boundMax - boundMin;
    return std::max({box.x, box.y, box.z});
  }

  void getScreenBounding(
    glm::ivec2& bMin, glm::ivec2& bMax, int H, int W, const glm::mat4& VP) const;

  inline const AssimpData* getMeshData() const {return data;}

  inline int getPolygonNum() const {
    int res = 0;
    for(const auto& mesh: data->meshes) {
      res += mesh.indices.size() / 3;
    }
    return res;
  }
};


class PointCloud: public ModelBase {
private:
  PlyData *plyData = nullptr;

public:
  PointCloud() {mType = ModelType::Points;}
  PointCloud(std::string const &path, unsigned int renderPrio = SOLID_PRIO): 
    ModelBase(Shader::GetPCShader(), renderPrio), plyData(new PlyData(path)){
    mType = ModelType::Points;
  }

  void draw(const Shader* s = nullptr) {
    plyData->draw();
  }

  void getBound(glm::vec3& minB, glm::vec3& maxB) const {
    minB = plyData->minBound;
    maxB = plyData->maxBound;
  }
};
