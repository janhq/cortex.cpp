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
};
}  // namespace commands