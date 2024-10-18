#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace commands {

struct ModelLoadedStatus {
  std::string engine;
  std::string model;
  uint64_t ram;
  uint64_t start_time;
  uint64_t vram;
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
