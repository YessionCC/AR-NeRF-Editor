#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

class Serializer {
public:
  static std::vector<Serializer*> series;

public:
  Serializer() {
    series.push_back(this);
  }
  virtual void dump(std::ofstream& file) = 0;
  virtual void load(std::ifstream& file) = 0;
  virtual ~Serializer() {}

  static void dumpAll(std::string path) {
    std::ofstream file(path);
    for(auto s: series) s->dump(file);
    file.close();
  }

  static void loadAll(std::string path) {
    std::ifstream file(path);
    if(!file.good()) {
      std::cout<<"Serializes file not found"<<std::endl;
      return;
    }
    for(auto s: series) s->load(file);
    file.close();
  }

};