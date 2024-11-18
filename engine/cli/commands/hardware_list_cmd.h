#pragma once
#include <optional>
#include <string>

namespace commands {
struct HarwareOptions {
  bool show_cpu = false;
  bool show_os = false;
  bool show_ram = false;
  bool show_storage = false;
  bool show_gpu = false;
  bool show_power = false;
  bool show_monitors = false;

  bool has_flag() const {
    return show_cpu || show_os || show_ram || show_storage || show_gpu ||
           show_power || show_monitors;
  }
};

class HardwareListCmd {
 public:
  bool Exec(const std::string& host, int port,
            const std::optional<HarwareOptions>& ho);
};
}  // namespace commands