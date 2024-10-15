#pragma once
#include <string>

namespace commands {
class EngineGetCmd {
 public:
  void Exec(const std::string& host, int port,
            const std::string& engineName) const;

};
}  // namespace commands
