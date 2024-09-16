#include <iostream>
#include "nlohmann/json.hpp"
#include "utils/engine_matcher_utils.h"
#include "utils/system_info_utils.h"

int main(int argc, char* argv[]) {
  trantor::Logger::setLogLevel(trantor::Logger::kFatal);
  auto system_info = system_info_utils::GetSystemInfo();
  cortex::cpuid::CpuInfo cpu_info;
  auto suitable_avx = engine_matcher_utils::GetSuitableAvxVariant(cpu_info);
  auto gpu_info_list = system_info_utils::GetGpuInfoList();

  nlohmann::ordered_json json_data;
  json_data["os"] = system_info.os;
  json_data["arch"] = system_info.arch;
  json_data["suitable_avx"] = suitable_avx;
  json_data["gpu_info"] = nlohmann::json::array();
  for (auto& gpu_info : gpu_info_list) {
    nlohmann::ordered_json info;
    info["id"] = gpu_info.id;
    info["name"] = gpu_info.name;
    info["arch"] = gpu_info.arch;
    info["driver_version"] = gpu_info.driver_version.has_value()
                                 ? gpu_info.driver_version.value()
                                 : "";
    info["cuda_driver_version"] = gpu_info.cuda_driver_version.has_value()
                                      ? gpu_info.cuda_driver_version.value()
                                      : "";
    info["compute_cap"] =
        gpu_info.compute_cap.has_value() ? gpu_info.compute_cap.value() : "";
    json_data["gpu_info"].push_back(info);
  }
  std::cout << json_data.dump() << std::endl;
  return 0;
}