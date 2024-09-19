#pragma once

#include <optional>
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
  constexpr static auto kIncompatible = "Incompatible";
  constexpr static auto kReady = "Ready";
  constexpr static auto kNotInstalled = "Not Installed";

  const std::vector<std::string_view> kSupportEngines = {
      "cortex.llamacpp", "cortex.onnx", "cortex.tensorrt-llm"};

  std::optional<EngineInfo> GetEngineInfo(const std::string& engine) const;

  std::vector<EngineInfo> GetEngineInfoList() const;

  void InstallEngine(const std::string& engine,
                     const std::string& version = "latest");

  void InstallLocalEngine(const std::string& engine, const std::string& path);

  void UninstallEngine(const std::string& engine);
};
