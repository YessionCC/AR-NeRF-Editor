#include "cpp_socket.hpp"
#include <unistd.h>

Connector::Connector(const std::string ip, int port) { Init(ip, port); }
Connector::~Connector() { close(_socket); }

void Connector::Init(const std::string ip, int port) {
  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket < 0) {
    std::cout << "[Client]: ERROR establishing socket\n" << std::endl;
    exit(1);
  }

  bool connected = false;
  int connection_attempts = 5;

  while ((!connected) && (connection_attempts > 0)) {
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);

    if (connect(_socket, (const struct sockaddr*)&serv_addr,
                sizeof(serv_addr)) == 0) {
      connected = true;
      std::cout << "[Client]: Cpp socket client connected." << std::endl;
    } else {
      port += 1;
      connection_attempts -= 1;
      std::cout << "[Client]: Error connecting to port " << port - 1
                << ". Attepting to connect to port: " << port << std::endl;
    }
  }
}
// length + content: length is in byte
void Connector::Send(char* msg, int64_t length) {
  send(_socket, &length, sizeof(int64_t), 0);
  send(_socket, msg, length, 0);
}

int64_t Connector::Receive(char* buf) {
  int64_t msg_length;
  int64_t len = recv(_socket, &msg_length, sizeof(int64_t), 0);
  if(len <= 0) return len; // nothing received, no block
  len = 0;
  while(len < msg_length) {
    int64_t rlen = recv(_socket, buf+len, msg_length, 0);
    if(rlen <= 0) {
      std::cout << "Error: no content message"<<std::endl;
      break;
    }
    len += rlen;
  }
  return msg_length;
}

int64_t Connector::Receive(char** buf) {
  int64_t msg_length;
  int64_t len = recv(_socket, &msg_length, sizeof(int64_t), 0);
  if(len <= 0) return len; // nothing received, no block
  *buf = new char[msg_length];
  len = 0;
  while(len < msg_length) {
    int64_t rlen = recv(_socket, (*buf)+len, msg_length, 0);
    if(rlen <= 0) {
      std::cout << "Error: no content message"<<std::endl;
      break;
    }
    len += rlen;
  }
  return msg_length;
}

