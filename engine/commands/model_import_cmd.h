#pragma once

#include <cmath>  // For std::isnan
#include <string>
namespace commands {

class ModelImportCmd {
 public:
  ModelImportCmd(std::string model_handle, std::string model_path);
  void Exec();

 private:
  std::string model_handle_;
  std::string model_path_;
};
}  // namespace commands