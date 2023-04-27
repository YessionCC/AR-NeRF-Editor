#pragma once


class BlenderCmp {
private:
  int save_idx = 0;

public:
  static BlenderCmp& getInstance() {
    static BlenderCmp bcmp;
    return bcmp;
  }

  void run_cmp();
};