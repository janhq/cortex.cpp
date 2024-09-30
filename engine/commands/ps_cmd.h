#pragma once

#include <string>
#include <vector>

namespace commands {

struct ModelLoadedStatus {
  std::string engine;
  std::string model;
  std::string ram;
  uint64_t start_time;
  std::string vram;
};

class PsCmd {
 public:
  explicit PsCmd() = default;

  void Exec(const std::string& host, int port);

 private:
  void PrintModelStatusList(
      const std::vector<ModelLoadedStatus>& model_status_list) const;
};

}  // namespace commands
