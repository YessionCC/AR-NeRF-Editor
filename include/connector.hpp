#pragma once

#include "cpp_socket.hpp"
#include "camera.hpp"

enum Action {
  ProbePos = 1,
  CamPose = 2,
  Maps = 3,
  Material = 4,
  ShadowField = 5,
  Render = 6,
  ShadowMap = 7,
  ShadowFieldPath = 8,
  SSDFPath = 9,
  SG_Use_SShadow = 10,
  Cmp_Methods = 11,
  Decomposition_Cmp = 12,
  UpdateSaveIndex = 13
};

struct SavePack {
std::string save_prefix;
bool is_save_info;
};

class ModelBase;
class NGPConnector {
private:
  Connector conn;

public:
  NGPConnector(const std::string ip, int port): conn(ip, port) {}
  void sendProbePos(std::vector<ModelBase*>& models);
  void sendCamPose();
  void sendMapsSimple(
    ViewportCamera& cam, std::vector<ModelBase*>& models, int W, int H, 
    bool use_sg_shader = false);
  void sendMapsComplex(
    ViewportCamera& cam, std::vector<ModelBase*>& models, int W, int H);
  void sendRenderCall();
  void sendRenderSaveCall(SavePack* sp);
  void sendMaterial();
  void sendShadowInfo(ViewportCamera& cam, int H, int W);
  void sendShadowMap();
  void sendShadowFieldInfo(std::string model_path);
  void sendSSDFInfo(std::string model_path);
  void sendSForSSDFInfo(std::string model_path);
  void sendSGUseSShadow();

  void sendMethodsCompareInfo();
  void sendProcessDecompositeCmpCall();
  void sendUpdateIndexCall();

  void recvHWF();
  void recvMainDirectionLight();
  void recvBlenderTrans();
  void recvRenderCompleteFlag();
  void recvSGLights();

  static NGPConnector* getConnector();
  static void setConnector(NGPConnector* conn);
};