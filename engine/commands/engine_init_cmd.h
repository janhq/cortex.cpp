#pragma once

#include <array>
#include <string>

namespace commands {

class EngineInitCmd {
 public:
  EngineInitCmd(std::string engineName, std::string version);

  bool Exec() const;

 private:
  std::string engineName_;
  std::string version_;

  static constexpr std::array<const char*, 3> supportedEngines_ = {
      "cortex.llamacpp", "cortex.onnx", "cortex.tensorrt-llm"};
};
}  // namespace commands