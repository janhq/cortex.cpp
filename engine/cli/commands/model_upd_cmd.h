#pragma once

#include <json/value.h>
#include <functional>
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
  void UpdateConfig(Json::Value& data, const std::string& key,
                    const std::string& value);
  void UpdateVectorField(
      const std::string& key, const std::string& value,
      std::function<void(const std::vector<std::string>&)> setter);
  void UpdateNumericField(const std::string& key, const std::string& value,
                          std::function<void(float)> setter);
  void UpdateBooleanField(const std::string& key, const std::string& value,
                          std::function<void(bool)> setter);

 private:
  std::string model_handle_;
};
}  // namespace commands
