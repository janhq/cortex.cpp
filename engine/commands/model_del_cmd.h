#pragma once

#include <string>

namespace commands {

class ModelDelCmd {
 public:
  bool Exec(const std::string& model_handle);
};
}