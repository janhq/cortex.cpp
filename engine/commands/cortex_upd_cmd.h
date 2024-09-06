#pragma once
#include <string>
#include <optional>

namespace commands {

class CortexUpdCmd{
 public:
  CortexUpdCmd();
  void Exec(std::string version);
};

}  // namespace commands