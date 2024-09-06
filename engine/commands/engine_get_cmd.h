#pragma once

#include <string>

namespace commands {
class EngineGetCmd {
 public:
  EngineGetCmd(const std::string& engine) : engine_{engine} {};

  void Exec() const;

 private:
  std::string engine_;
};
}  // namespace commands
