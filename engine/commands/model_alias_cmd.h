#pragma once
#include <string>
#include "utils/logging_utils.h"
namespace commands {

class ModelAliasCmd {
 public:
  void Exec(const std::string& model_handle, const std::string& model_alias);
};
}  // namespace commands