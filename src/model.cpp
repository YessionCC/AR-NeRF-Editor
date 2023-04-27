#include "model.hpp"
#include "texture.hpp"
#include "scene.hpp"
#include "utility.hpp"
#include "happly/happly.h"
#include "editor.hpp"

#include <fstream>

using namespace std;

void AssimpData::loadModel(string const &path) {
  // read file via ASSIMP
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  // check for errors
  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
  {
      cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
      return;
  }
  // retrieve the directory path of the filepath
  unsigned int slashpos = path.find_last_of('/');
  directory = path.substr(0, slashpos);
  modelName = path.substr(slashpos+1);

  // process ASSIMP's root node recursively
  processNode(scene->mRootNode, scene);
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void AssimpData::processNode(aiNode *node, const aiScene *scene) {
  // process each mesh located at the current node
  for(unsigned int i = 0; i < node->mNumMeshes; i++){
    // the node object only contains indices to index the actual objects in the scene. 
    // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }
  // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
  for(unsigned int i = 0; i < node->mNumChildren; i++){
    processNode(node->mChildren[i], scene);
  }

}

Mesh AssimpData::processMesh(aiMesh *mesh, const aiScene *scene) {
  // data to fill
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vector<ImageTexture> textures;

  float flmax = std::numeric_limits<float>::max();
  float flmin = std::numeric_limits<float>::min();
  glm::vec3 meshBoundMin = glm::vec3(flmax), meshBoundMax = glm::vec3(flmin);

  // walk through each of the mesh's vertices
  for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
    // positions
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.Position = vector;
    meshBoundMax = glm::max(meshBoundMax, vector);
    meshBoundMin = glm::min(meshBoundMin, vector);
    // normals
    if (mesh->HasNormals()) {
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.Normal = vector;
    }
    // texture coordinates
    if(mesh->mTextureCoords[0]) {// does the mesh contain texture coordinates?
      glm::vec2 vec;
      // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
      // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
      vec.x = mesh->mTextureCoords[0][i].x; 
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.TexCoords = vec;
      // tangent
      vector.x = mesh->mTangents[i].x;
      vector.y = mesh->mTangents[i].y;
      vector.z = mesh->mTangents[i].z;
      vertex.Tangent = vector;
      // bitangent
      vector.x = mesh->mBitangents[i].x;
      vector.y = mesh->mBitangents[i].y;
      vector.z = mesh->mBitangents[i].z;
      vertex.Bitangent = vector;
    }
    else
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    vertices.push_back(vertex);
  }
  // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
  for(unsigned int i = 0; i < mesh->mNumFaces; i++){
    aiFace face = mesh->mFaces[i];
    // retrieve all indices of the face and store them in the indices vector
    for(unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);        
  }
  // process materials
  aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
  // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
  // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
  // Same applies to other texture as the following list summarizes:
  // diffuse: texture_diffuseN
  // specular: texture_specularN
  // normal: texture_normalN

  // 1. diffuse maps
  vector<ImageTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
  // 2. specular maps
  vector<ImageTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  // 3. normal maps
  std::vector<ImageTexture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
  // 4. height maps
  std::vector<ImageTexture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
  textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
  
  // return a mesh object created from the extracted mesh data
  return Mesh(vertices, indices, textures, meshBoundMin, meshBoundMax);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<ImageTexture> AssimpData::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
  static std::vector<ImageTexture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
  vector<ImageTexture> textures;
  for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
    string path = this->directory+"/"+str.C_Str();
    bool skip = false;
    for(unsigned int j = 0; j < textures_loaded.size(); j++) {
      if(textures_loaded[j].path == path) {
        textures.push_back(textures_loaded[j]);
        skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
        break;
      }
    }
    if(!skip) {   // if texture hasn't been loaded already, load it
      ImageTexture texture(path, typeName);
      textures.push_back(texture);
      textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
    }
  }
  return textures;
}

void AssimpData::getBound(glm::vec3& boundMin, glm::vec3& boundMax) const {
  float flmax = std::numeric_limits<float>::max();
  float flmin = std::numeric_limits<float>::min();
  boundMin = glm::vec3(flmax), boundMax = glm::vec3(flmin);
  for(const Mesh& m: meshes) {
    boundMax = glm::max(boundMax, m.meshBoundMax);
    boundMin = glm::min(boundMin, m.meshBoundMin);
  }
}

void Model::getScreenBounding(
  glm::ivec2& bMin, glm::ivec2& bMax, int H, int W, const glm::mat4& VP) const {
  glm::vec3 bmin, bmax;
  data->getBound(bmin, bmax);
  glm::mat4 MVP = VP*Editor::getInstance().world_trans*transformMat;
  getBB3ScreenPos(bmin, bmax, bMin, bMax, MVP, H, W);
}

void PlyData::loadPly(std::string path) {
  printf("Read ply file...\n");
  happly::PLYData plyIn(path);
  std::vector<double> xs = plyIn.getElement("vertex").getProperty<double>("x");
  std::vector<double> ys = plyIn.getElement("vertex").getProperty<double>("y");
  std::vector<double> zs = plyIn.getElement("vertex").getProperty<double>("z");
  std::vector<u_char> red = plyIn.getElement("vertex").getProperty<u_char>("red");
  std::vector<u_char> green = plyIn.getElement("vertex").getProperty<u_char>("green");
  std::vector<u_char> blue = plyIn.getElement("vertex").getProperty<u_char>("blue");

  pointsNum = xs.size();
  minBound = glm::vec3(FLT_MAX);
  maxBound = glm::vec3(FLT_MIN);
  pcData.resize(pointsNum);
  for(size_t i = 0; i<pointsNum; i++) {
    pcData[i].point = glm::vec3((float)xs[i], (float)ys[i], (float)zs[i]);
    pcData[i].color = glm::vec3(red[i]/255.0f, green[i]/255.0f, blue[i]/255.0f);
    minBound = glm::min(minBound, pcData[i].point);
    maxBound = glm::max(maxBound, pcData[i].point);
  }
  printf("Read ply file complete\n");
  upload();
}

void PlyData::upload() {
  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(
    GL_ARRAY_BUFFER, pcData.size() * sizeof(glm::vec3), 
    pcData.data(), GL_STATIC_DRAW
  );  

  // pc Positions
  glEnableVertexAttribArray(0);	
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PosCol), (void*)0);
  // pc Colors
  glEnableVertexAttribArray(1);	
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PosCol), (void*)offsetof(PosCol, color));

  glBindVertexArray(0);
}

void PlyData::draw() {
  glBindVertexArray(VAO);
  glDrawArrays(GL_POINTS, 0, pointsNum);
  glBindVertexArray(0);
}

void Model::separateInMesh(std::vector<Model>& models) const {
  models.resize(data->meshes.size());
  for(unsigned int i = 0; i<data->meshes.size(); i++) {
    models[i] = *this;
    models[i].data = new AssimpData();
    models[i].data->subMeshID = i;
    models[i].data->directory = this->data->directory;
    models[i].data->modelName = this->data->modelName;
    models[i].data->gammaCorrection = this->data->gammaCorrection;
    models[i].data->meshes.push_back(std::move(this->data->meshes[i]));
  }
  std::cout<<"model: "<<this->data->directory<<" has separate succeesfully, "
    <<"with "<<(int)models.size()<<"sub meshes"<<std::endl;
}