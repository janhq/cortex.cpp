#include "system_info_utils.h"
#include "utils/logging_utils.h"

namespace system_info_utils {
std::pair<std::string, std::string> GetDriverAndCudaVersion() {
  if (!IsNvidiaSmiAvailable()) {
    CTL_INF("nvidia-smi is not available!");
    return {};
  }
  try {
    std::string driver_version;
    std::string cuda_version;
    CommandExecutor cmd("nvidia-smi");
    auto output = cmd.execute();

    const std::regex driver_version_reg(kDriverVersionRegex);
    std::smatch driver_match;

    if (std::regex_search(output, driver_match, driver_version_reg)) {
      LOG_INFO << "Gpu Driver Version: " << driver_match[1].str();
      driver_version = driver_match[1].str();
    } else {
      LOG_ERROR << "Gpu Driver not found!";
      return {};
    }

    const std::regex cuda_version_reg(kCudaVersionRegex);
    std::smatch cuda_match;

    if (std::regex_search(output, cuda_match, cuda_version_reg)) {
      LOG_INFO << "CUDA Version: " << cuda_match[1].str();
      cuda_version = cuda_match[1].str();
    } else {
      LOG_ERROR << "CUDA Version not found!";
      return {};
    }
    return std::pair(driver_version, cuda_version);
  } catch (const std::exception& e) {
    LOG_ERROR << "Error: " << e.what();
    return {};
  }
}

std::vector<GpuInfo> GetGpuInfoListVulkan() {
  std::vector<GpuInfo> gpuInfoList;

  try {
    // NOTE: current ly we don't have logic to download vulkaninfoSDK
#ifdef _WIN32
    CommandExecutor cmd("vulkaninfoSDK.exe --summary");
#else
    CommandExecutor cmd("vulkaninfoSDK --summary");
#endif
    auto output = cmd.execute();

    // Regular expression patterns to match each field
    std::regex gpu_block_reg(R"(GPU(\d+):)");
    std::regex field_pattern(R"(\s*(\w+)\s*=\s*(.*))");

    std::sregex_iterator iter(output.begin(), output.end(), gpu_block_reg);
    std::sregex_iterator end;

    while (iter != end) {
      GpuInfo gpuInfo;

      // Extract GPU ID from the GPU block pattern (e.g., GPU0 -> id = "0")
      gpuInfo.id = (*iter)[1].str();

      auto gpu_start_pos = iter->position(0) + iter->length(0);
      auto gpu_end_pos = std::next(iter) != end ? std::next(iter)->position(0)
                                                : std::string::npos;
      std::string gpu_block =
          output.substr(gpu_start_pos, gpu_end_pos - gpu_start_pos);

      std::sregex_iterator field_iter(gpu_block.begin(), gpu_block.end(),
                                      field_pattern);

      while (field_iter != end) {
        std::string key = (*field_iter)[1].str();
        std::string value = (*field_iter)[2].str();

        if (key == "deviceName")
          gpuInfo.name = value;
        else if (key == "apiVersion")
          gpuInfo.compute_cap = value;

        gpuInfo.vram_total = "";  // not available
        gpuInfo.arch = GetGpuArch(gpuInfo.name);

        ++field_iter;
      }

      gpuInfoList.push_back(gpuInfo);
      ++iter;
    }
  } catch (const std::exception& e) {
    LOG_ERROR << "Error: " << e.what();
  }

  return gpuInfoList;
}

std::vector<GpuInfo> GetGpuInfoList() {
  std::vector<GpuInfo> gpuInfoList;
  if (!IsNvidiaSmiAvailable())
    return gpuInfoList;
  try {
    auto [driver_version, cuda_version] = GetDriverAndCudaVersion();
    if (driver_version.empty() || cuda_version.empty())
      return gpuInfoList;
    bool need_fallback = false;
    CommandExecutor cmd(kGpuQueryCommand);
    auto output = cmd.execute();
    if (output.find("NVIDIA") == std::string::npos) {
      need_fallback = true;
      output = CommandExecutor(kGpuQueryCommandFb).execute();
    }

    std::string rg = need_fallback ? kGpuInfoRegexFb : kGpuInfoRegex;
    const std::regex gpu_info_reg(rg);
    std::smatch match;
    std::string::const_iterator search_start(output.cbegin());
    int rg_count = need_fallback ? 5 : 6;

    while (
        std::regex_search(search_start, output.cend(), match, gpu_info_reg)) {
      GpuInfo gpuInfo = {match[1].str(),              // id
                         match[2].str(),              // vram_total
                         match[3].str(),              // vram_free
                         match[4].str(),              // name
                         GetGpuArch(match[4].str()),  // arch
                         driver_version,              // driver_version
                         cuda_version,                // cuda_driver_version
                         need_fallback ? "0" : match[5].str(),  // compute_cap
                         match[rg_count].str(),                 // uuid
                         "NVIDIA"};
      gpuInfoList.push_back(gpuInfo);
      search_start = match.suffix().first;
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return gpuInfoList;
}
}  // namespace system_info_utils