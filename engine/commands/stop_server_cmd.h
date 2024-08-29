#pragma once
#include <string>

namespace commands {

class StopServerCmd{
 public:
  StopServerCmd(std::string host, int port);
  void Exec();

 private:
  std::string host_;
  int port_;
};
}  // namespace commands