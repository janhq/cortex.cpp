#pragma once
#include <string>

namespace commands {

class ServerStartCmd{
 public:
  ServerStartCmd(std::string host, int port);
  void Exec();

 private:
  std::string host_;
  int port_;
};
}  // namespace commands