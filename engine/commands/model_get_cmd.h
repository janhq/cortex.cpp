#pragma once


#include <cmath>  // For std::isnan
#include <string>
namespace commands {

class ModelGetCmd {
 public:

  ModelGetCmd(std::string model_handle);
  void Exec();

 private:
  std::string model_handle_;
};
}  // namespace commands