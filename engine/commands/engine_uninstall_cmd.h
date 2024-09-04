#pragma once
#include <array>
#include <string>

namespace commands {
class EngineUninstallCmd {
 public:
  EngineUninstallCmd(std::string engine);

  void Exec() const;

 private:
  std::string engine_;

  static constexpr std::array<const char*, 3> supportedEngines_ = {
      "cortex.llamacpp", "cortex.onnx", "cortex.tensorrt-llm"};
};
}  // namespace commands
