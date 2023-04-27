#pragma once

#include <string>
#include "connector.hpp"


class PathMan {
public:
  int currentSaveID = 0;
  std::string curScene = "";
  std::string result_path_base = "/home/lofr/Projects/NeRF/ngp_pl-master/insert/generate/";
  std::string blender_scene_base = "./blender_file/";//"/media/lofr/Develop/Develop/Blenders/scenes/";
  std::string IRAdobe_base = "/home/lofr/Projects/NeRF/InverseRenderingOfIndoorScene-master/";

public:
  static PathMan& getIns() {
    static PathMan pm;
    return pm;
  }

  void SetCurScene(std::string cs) {
    curScene = cs;
  }

  std::string ScenePath() {
    return result_path_base + curScene + "/";
  }

  std::string BlenderScenePath() {
    return blender_scene_base + curScene + "/";
  }

  std::string BlenderSceneFilePath() {
    return blender_scene_base + curScene + ".blend";
  }

  std::string ResultPath() {
    return ScenePath() + "results/";
  }

  std::string GetEnvImResultPath() {
    return ResultPath()+std::to_string(currentSaveID)+"_env.png";
  }

  // set -1 to allow auto increase
  void updateSaveInfos(int target_index) {
    if(target_index == -1)
      currentSaveID ++;
    else  
      currentSaveID = target_index;
    NGPConnector::getConnector()->sendUpdateIndexCall();
  }
};