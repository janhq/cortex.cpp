#pragma once

#include <string>
namespace commands {

class ModelImportCmd {
 public:
  void Exec(const std::string& host, int port, const std::string& model_handle,
            const std::string& model_path);
};
}  // namespace commands