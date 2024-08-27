#pragma once

#include <array>
#include <string>

namespace commands {

class EngineInitCmd {
 public:
  EngineInitCmd(std::string engineName, std::string version);

  void Exec() const;

 private:
  std::string engineName_;
  std::string version_;

  static constexpr std::array<const char*, 1> supportedEngines_ = {
      "cortex.llamacpp"};
};
}  // namespace commands