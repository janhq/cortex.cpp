#pragma once

#include <string>

namespace commands {

class ModelPullCmd {
 public:
explicit  ModelPullCmd(std::string model_handle, std::string branch);
  bool Exec();

 private:
  std::string model_handle_;
  std::string branch_;
};
}  // namespace commands