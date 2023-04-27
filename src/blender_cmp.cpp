#include "blender_cmp.hpp"
#include "scene.hpp"
#include "editor.hpp"
#include "pathManager.hpp"
#include "serializer.hpp"
#include <sstream>
#include <fstream>

std::string Mat4toPyArrayStr(glm::mat4& mat) {
  std::stringstream ss;
  ss << "[["<<mat[0][0]<<","<<mat[0][1]<<","<<mat[0][2]<<","<<mat[0][3]<<"],"
    << "["<<mat[1][0]<<","<<mat[1][1]<<","<<mat[1][2]<<","<<mat[1][3]<<"],"
    << "["<<mat[2][0]<<","<<mat[2][1]<<","<<mat[2][2]<<","<<mat[2][3]<<"],"
    << "["<<mat[3][0]<<","<<mat[3][1]<<","<<mat[3][2]<<","<<mat[3][3]<<"]]";
  return ss.str();
}

std::string Vec3toPyArrayStr(glm::vec3 vec) {
  std::stringstream ss;
  ss << "["<<vec[0]<<","<<vec[1]<<","<<vec[2]<<"]";
  return ss.str();
}

void BlenderCmp::run_cmp() {
  ShaderInfo& sInfo = Scene::CurrentScene()->GetCurrentShaderInfo();
  Editor& editor = Editor::getInstance();
  PathMan& pm = PathMan::getIns();

  std::string saveID = std::to_string(pm.currentSaveID);

  glm::mat4 pose = sInfo.invV;
  editor.getCamTransInRawBlenderScene(pose);
  pose = glm::transpose(pose);

  std::string blender_py_code = 
  "import bpy\n"
  "from mathutils import Matrix, Vector, Euler\n"
  "import numpy as np\n"
  "obj = bpy.data.objects[\'VO\']\n"
  "cam = bpy.data.objects[\'Camera\']\n"
  "cam.matrix_world = Matrix("+Mat4toPyArrayStr(pose)+")\n"
  "obj.scale = " + Vec3toPyArrayStr(glm::vec3(editor.getObjScaleInRawBlenderScene())) + "\n"
  "bpy.context.object.rotation_mode = \'ZYX\'\n"
  "obj.rotation_euler = Euler(" + Vec3toPyArrayStr(editor.model_rotate) + ")\n"
  "obj.location = " + Vec3toPyArrayStr(editor.getObjTransInRawBlenderScene()) + "\n"
  "bpy.data.materials[\'VO\'].node_tree.nodes[\'Glossy BSDF\'].inputs[1].default_value = "+std::to_string(pow(editor.model_rough, 0.8)) + "\n"  // rough
  "bpy.data.materials[\'VO\'].node_tree.nodes[\'Mix Shader\'].inputs[0].default_value = "+std::to_string(pow(editor.model_metal, 0.04)) + "\n"  // metal
  "bpy.data.scenes[\'Scene\'].render.filepath = \'"+pm.ResultPath()+saveID+"_blender"+".exr\'\n"
  "bpy.ops.render.render(write_still=True)"
  ;
  std::ofstream pyfile("blender_cmd.py");
  pyfile << blender_py_code;
  pyfile.close();

  std::ofstream pyfileBU(pm.ResultPath()+saveID + "_blender_cmd"+".py");
  pyfileBU << blender_py_code;
  pyfileBU.close();

  Serializer::dumpAll(pm.ResultPath()+saveID + "_viewer_config"+".ini");

  std::cout << "Run Blender" << std::endl;
  
  std::string _cmd = "./blender_file/blender "+
    pm.BlenderSceneFilePath()+" --background --python blender_cmd.py";
  system(_cmd.c_str());
}