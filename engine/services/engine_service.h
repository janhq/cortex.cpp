#pragma once

#include <string>
#include <string_view>
#include <vector>

struct EngineInfo {
  std::string name;
  std::string description;
  std::string format;
  std::string version;
  std::string product_name;
  std::string status;
};

class EngineService {
 public:
  const std::vector<std::string> kSupportEngines = {
      "cortex.llamacpp", "cortex.onnx", "cortex.tensorrt-llm"};

  EngineInfo GetEngineInfo(const std::string& engine) const;

  std::vector<EngineInfo> GetEngineInfoList() const;

  void InstallEngine(const std::string& engine);

  void UninstallEngine(const std::string& engine);
};
