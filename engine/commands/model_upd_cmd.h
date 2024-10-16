#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace commands {
class ModelUpdCmd {
 public:
  ModelUpdCmd(std::string model_handle);

  void Exec(const std::string& host, int port,
            const std::unordered_map<std::string, std::string>& options);
 private:
  std::string model_handle_;
};
}  // namespace commands