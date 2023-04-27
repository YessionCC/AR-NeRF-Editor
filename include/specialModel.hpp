#pragma once

#include <vector>
#include <string>

#include "model.hpp"

class SpecialModel: public ModelBase {
private:
  int vtxNumToDraw;
  unsigned int VAO;

public:
  virtual ~SpecialModel() {}
  SpecialModel(): ModelBase() {}
  SpecialModel(
    const Shader* s, unsigned int renderprio, unsigned int VAO, int vtxNum
  ): ModelBase(s, renderprio), vtxNumToDraw(vtxNum), VAO(VAO){}

  virtual void draw(const Shader* s = nullptr) override;
};

class Grid: public SpecialModel {
private:
  float scale;

public:
  Grid(float scale = 1.0f);

  void draw(const Shader* s = nullptr) override{
    assert(s == nullptr || s == shader);
    SpecialModel::draw(s);
  }

  void beforeDraw() override {
    shader->setFloat("scale", scale);
  }
};



class Skybox: public SpecialModel {
public:
  CubeMapTexture cubemap;
  RenderTexture* cpRT = nullptr;

public:
  Skybox(std::vector<std::string> tex_face_paths);

  inline unsigned int getCubeMapID() const {return cubemap.id;}
  void beforeDraw() override;
};


class Plane: public SpecialModel {
public:
  Plane(const Shader* s, unsigned int prio);
};