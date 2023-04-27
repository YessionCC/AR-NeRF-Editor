#pragma once

#pragma once

#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>


class Connector {
 public:
  Connector() {}
  Connector(const std::string ip, int port);
  ~Connector();

  void Init(const std::string ip = "127.0.0.1", int port = 5001);

  void Send(char* msg, int64_t length);

  // make sure the buffer is large enough, return == 0 means nothing recv
  int64_t Receive(char* buf);
  // need to release buffer manually!
  int64_t Receive(char** buf);

 private:
  int _socket;
  const int size_message_length_ = 16;  // Buffer size for the length
};