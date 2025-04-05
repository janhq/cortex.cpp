#pragma once
#include <optional>
#include <string>
#include <array>

namespace commands {
struct HardwareQueryFlags {
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
  bool Exec(const std::string& server_host, int server_port,
            const std::optional<HardwareQueryFlags>& query_flags);

 private:
  // Static constexpr arrays for column headers
  static constexpr std::array<const char*, 6> CPU_INFO_HEADERS = {
      "#", "Arch", "Cores", "Model", "Usage", "Instructions"};
  static constexpr std::array<const char*, 3> OS_INFO_HEADERS = {"#", "Version",
                                                            "Name"};
  static constexpr std::array<const char*, 3> RAM_INFO_HEADERS = {"#", "Total (MiB)",
                                                             "Available (MiB)"};
  static constexpr std::array<const char*, 9> GPU_INFO_HEADERS = {
      "#",           "GPU ID",          "Name",           "Version",
      "Total (MiB)", "Available (MiB)", "Driver Version", "Compute Capability",
      "Activated"};
  static constexpr std::array<const char*, 3> STORAGE_INFO_HEADERS = {
      "#", "Total (GiB)", "Available (GiB)"};
  static constexpr std::array<const char*, 4> POWER_INFO_HEADERS = {
      "#", "Battery Life", "Charging Status", "Power Saving"};
};
}  // namespace commands