#include "hardware_list_cmd.h"

#include <json/reader.h>
#include <json/value.h>
#include <iostream>

#include <vector>
#include "httplib.h"
#include "server_start_cmd.h"
#include "utils/curl_utils.h"
#include "utils/hardware/cpu_info.h"
#include "utils/hardware/gpu_info.h"
#include "utils/hardware/os_info.h"
#include "utils/hardware/power_info.h"
#include "utils/hardware/ram_info.h"
#include "utils/hardware/storage_info.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"
// clang-format off
#include <tabulate/table.hpp>
// clang-format on

namespace commands {
using namespace tabulate;
using Row_t =
    std::vector<variant<std::string, const char*, string_view, Table>>;

bool HardwareListCmd::Exec(const std::string& host, int port,
                           const std::optional<HarwareOptions>& ho) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return false;
    }
  }

  auto url = url_parser::Url{
      .protocol = "http",
      .host = host + ":" + std::to_string(port),
      .pathParams = {"v1", "hardware"},
  };
  auto result = curl_utils::SimpleGetJson(url.ToFullPath());
  if (result.has_error()) {
    CTL_ERR(result.error());
    return false;
  }

  if (!ho.has_value() || ho.value().show_cpu) {
    std::cout << "CPU Information:" << std::endl;
    Table table;
    std::vector<std::string> column_headers{"(Index)", "Arch", "Cores", "Model",
                                            "Instructions"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    std::vector<std::string> row = {"1"};
    hardware::CPU cpu = hardware::cpu::FromJson(result.value()["cpu"]);
    row.emplace_back(cpu.arch);
    row.emplace_back(std::to_string(cpu.cores));
    row.emplace_back(cpu.model);
    std::string insts;
    for (auto const& i : cpu.instructions) {
      insts += i + " ";
    };
    row.emplace_back(insts);
    table.add_row({row.begin(), row.end()});
    std::cout << table << std::endl;
    std::cout << std::endl;
  }

  if (!ho.has_value() || ho.value().show_os) {
    std::cout << "OS Information:" << std::endl;
    Table table;
    std::vector<std::string> column_headers{"(Index)", "Version", "Name"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    std::vector<std::string> row = {"1"};
    hardware::OS os = hardware::os::FromJson(result.value()["os"]);
    row.emplace_back(os.version);
    row.emplace_back(os.name);
    table.add_row({row.begin(), row.end()});
    std::cout << table << std::endl;
    std::cout << std::endl;
  }

  if (!ho.has_value() || ho.value().show_ram) {
    std::cout << "RAM Information:" << std::endl;
    Table table;
    std::vector<std::string> column_headers{"(Index)", "Total (MiB)",
                                            "Available (MiB)"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    std::vector<std::string> row = {"1"};
    hardware::Memory m = hardware::memory::FromJson(result.value()["ram"]);
    row.emplace_back(std::to_string(m.total_MiB));
    row.emplace_back(std::to_string(m.available_MiB));
    table.add_row({row.begin(), row.end()});
    std::cout << table << std::endl;
    std::cout << std::endl;
  }

 if (!ho.has_value() || ho.value().show_gpu) {
    std::cout << "GPU Information:" << std::endl;
    Table table;
    std::vector<std::string> column_headers{
        "(Index)",        "ID",
        "Name",           "Version",
        "Total (MiB)",    "Available (MiB)",
        "Driver Version", "Compute Capability"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    int count = 1;

    std::vector<hardware::GPU> gpus =
        hardware::gpu::FromJson(result.value()["gpus"]);
    for (auto const& gpu : gpus) {
      std::vector<std::string> row = {std::to_string(count)};
      row.emplace_back(gpu.id);
      row.emplace_back(gpu.name);
      row.emplace_back(gpu.version);
      row.emplace_back(std::to_string(gpu.total_vram));
      row.emplace_back(std::to_string(gpu.free_vram));
      row.emplace_back(
          std::get<hardware::NvidiaAddInfo>(gpu.add_info).driver_version);
      row.emplace_back(
          std::get<hardware::NvidiaAddInfo>(gpu.add_info).compute_cap);
      table.add_row({row.begin(), row.end()});
    }

    std::cout << table << std::endl;
    std::cout << std::endl;
  }

  if (!ho.has_value() || ho.value().show_storage) {
    std::cout << "Storage Information:" << std::endl;
    Table table;
    std::vector<std::string> column_headers{"(Index)", "Total (GiB)",
                                            "Available (GiB)"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    std::vector<std::string> row = {"1"};
    hardware::StorageInfo si =
        hardware::storage::FromJson(result.value()["storage"]);
    row.emplace_back(std::to_string(si.total));
    row.emplace_back(std::to_string(si.available));
    table.add_row({row.begin(), row.end()});
    std::cout << table << std::endl;
    std::cout << std::endl;
  }

  if (!ho.has_value() || ho.value().show_power) {
    std::cout << "Power Information:" << std::endl;
    Table table;
    std::vector<std::string> column_headers{"(Index)", "Battery Life",
                                            "Charging Status", "Power Saving"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    std::vector<std::string> row = {"1"};
    hardware::PowerInfo pi = hardware::power::FromJson(result.value()["power"]);
    row.emplace_back(std::to_string(pi.battery_life));
    row.emplace_back(pi.charging_status);
    row.emplace_back(pi.is_power_saving ? "Yes" : "No");
    table.add_row({row.begin(), row.end()});
    std::cout << table << std::endl;
    std::cout << std::endl;
  }

  return true;
}
}  // namespace commands