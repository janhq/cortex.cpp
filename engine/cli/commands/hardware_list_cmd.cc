#include "hardware_list_cmd.h"
#include <json/reader.h>
#include <json/value.h>
#include <iostream>
#include <vector>
#include "server_start_cmd.h"
#include "services/hardware_service.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
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
    std::vector<std::string> column_headers{"#",     "Arch",  "Cores",
                                            "Model", "Usage", "Instructions"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    std::vector<std::string> row = {"1"};
    cortex::hw::CPU cpu = cortex::hw::cpu::FromJson(result.value()["cpu"]);
    row.emplace_back(cpu.arch);
    row.emplace_back(std::to_string(cpu.cores));
    row.emplace_back(cpu.model);
    row.emplace_back(std::to_string(cpu.usage));
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
    std::vector<std::string> column_headers{"#", "Version", "Name"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    std::vector<std::string> row = {"1"};
    cortex::hw::OS os = cortex::hw::os::FromJson(result.value()["os"]);
    row.emplace_back(os.version);
    row.emplace_back(os.name);
    table.add_row({row.begin(), row.end()});
    std::cout << table << std::endl;
    std::cout << std::endl;
  }

  if (!ho.has_value() || ho.value().show_ram) {
    std::cout << "RAM Information:" << std::endl;
    Table table;
    std::vector<std::string> column_headers{"#", "Total (MiB)",
                                            "Available (MiB)"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    std::vector<std::string> row = {"1"};
    cortex::hw::Memory m = cortex::hw::memory::FromJson(result.value()["ram"]);
    row.emplace_back(std::to_string(m.total_MiB));
    row.emplace_back(std::to_string(m.available_MiB));
    table.add_row({row.begin(), row.end()});
    std::cout << table << std::endl;
    std::cout << std::endl;
  }

  if (!ho.has_value() || ho.value().show_gpu) {
    std::cout << "GPU Information:" << std::endl;
    Table table;
    std::vector<std::string> column_headers{"#",
                                            "GPU ID",
                                            "Name",
                                            "Version",
                                            "Total (MiB)",
                                            "Available (MiB)",
                                            "Driver Version",
                                            "Compute Capability",
                                            "Activated"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    int count = 1;

    std::vector<cortex::hw::GPU> gpus =
        cortex::hw::gpu::FromJson(result.value()["gpus"]);
    for (auto const& gpu : gpus) {
      std::vector<std::string> row = {std::to_string(count)};
      row.emplace_back(gpu.id);
      row.emplace_back(gpu.name);
      row.emplace_back(gpu.version);
      row.emplace_back(std::to_string(gpu.total_vram));
      row.emplace_back(std::to_string(gpu.free_vram));
      row.emplace_back(
          std::get<cortex::hw::NvidiaAddInfo>(gpu.add_info).driver_version);
      row.emplace_back(
          std::get<cortex::hw::NvidiaAddInfo>(gpu.add_info).compute_cap);
      row.emplace_back(gpu.is_activated ? "Yes" : "No");
      table.add_row({row.begin(), row.end()});
      count++;
    }

    std::cout << table << std::endl;
    std::cout << std::endl;
  }

  if (!ho.has_value() || ho.value().show_storage) {
    std::cout << "Storage Information:" << std::endl;
    Table table;
    std::vector<std::string> column_headers{"#", "Total (GiB)",
                                            "Available (GiB)"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    std::vector<std::string> row = {"1"};
    cortex::hw::StorageInfo si =
        cortex::hw::storage::FromJson(result.value()["storage"]);
    row.emplace_back(std::to_string(si.total));
    row.emplace_back(std::to_string(si.available));
    table.add_row({row.begin(), row.end()});
    std::cout << table << std::endl;
    std::cout << std::endl;
  }

  if (!ho.has_value() || ho.value().show_power) {
    std::cout << "Power Information:" << std::endl;
    Table table;
    std::vector<std::string> column_headers{"#", "Battery Life",
                                            "Charging Status", "Power Saving"};

    Row_t header{column_headers.begin(), column_headers.end()};
    table.add_row(header);
    table.format().font_color(Color::green);
    std::vector<std::string> row = {"1"};
    cortex::hw::PowerInfo pi =
        cortex::hw::power::FromJson(result.value()["power"]);
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
