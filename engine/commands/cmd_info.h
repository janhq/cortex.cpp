#pragma once
#include <string>
namespace commands {
struct CmdInfo {
  explicit CmdInfo(std::string model_id);

  std::string engine_name;
  std::string model_name;
  std::string branch;

 private:
  void Parse(std::string model_id);
};
}  // namespace commands