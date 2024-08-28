#pragma once
#include <string>
namespace commands {
struct CmdInfo {
  explicit CmdInfo(std::string model_id);

  std::string engine;
  std::string name;
  std::string branch;

 private:
  void Parse(std::string model_id);
};
}  // namespace commands