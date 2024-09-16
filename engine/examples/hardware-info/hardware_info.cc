#include <iostream>
#include "utils/engine_matcher_utils.h"
#include "utils/system_info_utils.h"

int main(int argc, char* argv[]) {
  auto patch = [](const std::string& s) -> std::string {
    return "\"" + s + "\"";
  };
  auto system_info = system_info_utils::GetSystemInfo();
  cortex::cpuid::CpuInfo cpu_info;
  auto suitable_avx = engine_matcher_utils::GetSuitableAvxVariant(cpu_info);
  auto nvidia_driver = system_info_utils::GetDriverVersion();
  auto cuda_version = system_info_utils::GetCudaVersion();

  std::cout << "{" << std::endl;
  std::cout << "\"os\": " << patch(system_info.os) << "," << std::endl;

  std::cout << "\"arch\": " << patch(system_info.arch) << "," << std::endl;
  std::cout << "\"suitable_avx\": " << patch(suitable_avx) << "," << std::endl;
  std::cout << "\"nvidia_driver\": " << patch(nvidia_driver) << ","
            << std::endl;
  std::cout << "\"cuda_version\": " << patch(cuda_version) << std::endl;
  std::cout << "}" << std::endl;
  return 0;
}