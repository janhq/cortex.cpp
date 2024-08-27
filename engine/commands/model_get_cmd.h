#pragma once

#include <string>
#include <cmath>    // For std::isnan
namespace commands {

class ModelGetCmd {
 public:
  ModelGetCmd(std::string modelHandle);
  void Exec();

 private:
  std::string modelHandle_;
};
}  // namespace commands