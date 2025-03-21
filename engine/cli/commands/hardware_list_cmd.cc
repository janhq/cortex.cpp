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
#include<numeric>
// clang-format on

namespace commands {
using Row_t = std::vector<
    variant<std::string, const char*, string_view, tabulate::Table>>;

bool HardwareListCmd::Exec(
    const std::string& host, int port,
    const std::optional<HardwareQueryFlags>& query_flags) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return false;
    }
  }

  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "hardware"},
      /* .queries = */ {},
  };

  auto hardware_json_response = curl_utils::SimpleGetJson(url.ToFullPath());
  if (hardware_json_response.has_error()) {
    CTL_ERR(hardware_json_response.error());
    return false;
  }

  // CPU Section
  if (!query_flags.has_value() || query_flags.value().show_cpu) {
    std::cout << "CPU Information:" << std::endl;
    tabulate::Table cpu_table;
    cpu_table.add_row(Row_t(CPU_INFO_HEADERS.begin(), CPU_INFO_HEADERS.end()));
    cpu_table.format()
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center)
        .padding_left(1)
        .padding_right(1);

    cortex::hw::CPU cpu =
        cortex::hw::cpu::FromJson(hardware_json_response.value()["cpu"]);
    std::vector<std::string> cpu_row = {
        "1",
        cpu.arch,
        std::to_string(cpu.cores),
        cpu.model,
        std::to_string(cpu.usage),
        std::accumulate(cpu.instructions.begin(), cpu.instructions.end(),
                        std::string{},
                        [](const std::string& a, const std::string& b) {
                          return a + (a.empty() ? "" : " ") + b;
                        })};
    cpu_table.add_row(Row_t(cpu_row.begin(), cpu_row.end()));
    std::cout << cpu_table << std::endl << std::endl;
  }

  // OS Section
  if (!query_flags.has_value() || query_flags.value().show_os) {
    std::cout << "OS Information:" << std::endl;
    tabulate::Table os_table;
    os_table.add_row(Row_t(OS_INFO_HEADERS.begin(), OS_INFO_HEADERS.end()));
    os_table.format()
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center)
        .padding_left(1)
        .padding_right(1);

    cortex::hw::OS os =
        cortex::hw::os::FromJson(hardware_json_response.value()["os"]);
    std::vector<std::string> os_row = {"1", os.version, os.name};
    os_table.add_row(Row_t(os_row.begin(), os_row.end()));
    std::cout << os_table << std::endl << std::endl;
  }

  // RAM Section
  if (!query_flags.has_value() || query_flags.value().show_ram) {
    std::cout << "RAM Information:" << std::endl;
    tabulate::Table ram_table;
    ram_table.add_row(Row_t(RAM_INFO_HEADERS.begin(), RAM_INFO_HEADERS.end()));
    ram_table.format()
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center)
        .padding_left(1)
        .padding_right(1);

    cortex::hw::Memory ram =
        cortex::hw::memory::FromJson(hardware_json_response.value()["ram"]);
    std::vector<std::string> ram_row = {"1", std::to_string(ram.total_MiB),
                                        std::to_string(ram.available_MiB)};
    ram_table.add_row(Row_t(ram_row.begin(), ram_row.end()));
    std::cout << ram_table << std::endl << std::endl;
  }

  // GPU Section
  if (!query_flags.has_value() || query_flags.value().show_gpu) {
    std::cout << "GPU Information:" << std::endl;
    tabulate::Table gpu_table;
    gpu_table.add_row(Row_t(GPU_INFO_HEADERS.begin(), GPU_INFO_HEADERS.end()));
    gpu_table.format()
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center)
        .padding_left(1)
        .padding_right(1);

    std::vector<cortex::hw::GPU> gpus =
        cortex::hw::gpu::FromJson(hardware_json_response.value()["gpus"]);
    int gpu_index = 1;
    for (const auto& gpu : gpus) {
      std::vector<std::string> gpu_row = {
          std::to_string(gpu_index),
          gpu.id,
          gpu.name,
          gpu.version,
          std::to_string(gpu.total_vram),
          std::to_string(gpu.free_vram),
          std::get<cortex::hw::NvidiaAddInfo>(gpu.add_info).driver_version,
          std::get<cortex::hw::NvidiaAddInfo>(gpu.add_info).compute_cap,
          gpu.is_activated ? "Yes" : "No"};
      gpu_table.add_row(Row_t(gpu_row.begin(), gpu_row.end()));
      gpu_index++;
    }
    std::cout << gpu_table << std::endl << std::endl;
  }
  
  // Storage Section
  if (!query_flags.has_value() || query_flags.value().show_storage) {
    std::cout << "Storage Information:" << std::endl;
    tabulate::Table storage_table;
    storage_table.add_row(Row_t(STORAGE_INFO_HEADERS.begin(), STORAGE_INFO_HEADERS.end()));
    storage_table.format()
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center)
        .padding_left(1)
        .padding_right(1);

    cortex::hw::StorageInfo storage = cortex::hw::storage::FromJson(
        hardware_json_response.value()["storage"]);
    std::vector<std::string> storage_row = {"1", std::to_string(storage.total),
                                            std::to_string(storage.available)};
    storage_table.add_row(Row_t(storage_row.begin(), storage_row.end()));
    std::cout << storage_table << std::endl << std::endl;
  }
  
  // Power Section
  if (!query_flags.has_value() || query_flags.value().show_power) {
    std::cout << "Power Information:" << std::endl;
    tabulate::Table power_table;
    power_table.add_row(Row_t(POWER_INFO_HEADERS.begin(), POWER_INFO_HEADERS.end()));
    power_table.format()
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center)
        .padding_left(1)
        .padding_right(1);

    cortex::hw::PowerInfo power =
        cortex::hw::power::FromJson(hardware_json_response.value()["power"]);
    std::vector<std::string> power_row = {
        "1", std::to_string(power.battery_life), power.charging_status,
        power.is_power_saving ? "Yes" : "No"};
    power_table.add_row(Row_t(power_row.begin(), power_row.end()));
    std::cout << power_table << std::endl << std::endl;
  }

  return true;
}
}  // namespace commands
