#pragma once

#include <string>

namespace commands {

class ModelPullCmd {
 public:
  ModelPullCmd(std::string modelHandle);
  void Exec();

 private:
  std::string modelHandle_;
};
}  // namespace commands