#pragma once

#include <string>
namespace commands {

class ModelGetCmd {
 public:
  void Exec(const std::string& model_handle);
};
}  // namespace commands
