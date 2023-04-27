#pragma once


#include <string>
#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.hpp"

#define MAX_BONE_INFLUENCE 4

class Shader;

struct Vertex {
  // position
  glm::vec3 Position;
  // normal
  glm::vec3 Normal;
  // texCoords
  glm::vec2 TexCoords;
  // tangent
  glm::vec3 Tangent;
  // bitangent
  glm::vec3 Bitangent;

  glm::vec3 PrtDiffSH9_1;
  glm::vec3 PrtDiffSH9_2;
  glm::vec3 PrtDiffSH9_3;
	//bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

class Mesh { // Mesh is the minimum unit can be drawn
public:
  // mesh Data
  std::vector<Vertex>       vertices;
  std::vector<unsigned int> indices;
  std::vector<ImageTexture>      textures;

  glm::vec3 meshBoundMin;
  glm::vec3 meshBoundMax;
  unsigned int VAO;

  // constructor
  Mesh(std::vector<Vertex> vertices, 
      std::vector<unsigned int> indices, 
      std::vector<ImageTexture> textures, 
      glm::vec3 boundMin = {0,0,0}, 
      glm::vec3 boundMax = {0,0,0});
  Mesh(){}

  // render the mesh
  void Draw(const Shader &shader) const;

private:
  // render data 
  unsigned int VBO, EBO;

  // initializes all the buffer objects/arrays
  void setupMesh();
};