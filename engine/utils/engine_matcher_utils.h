#include <trantor/utils/Logger.h>
#include <algorithm>
#include <iterator>
#include <regex>
#include <string>
#include <vector>
#include "utils/cpuid/cpu_info.h"
#include "utils/logging_utils.h"

namespace engine_matcher_utils {
inline std::string GetSuitableAvxVariant(cortex::cpuid::CpuInfo& cpu_info) {
  CTL_INF("GetSuitableAvxVariant:" << "\n" << cpu_info.to_string());

  // prioritize avx512 > avx2 > avx > noavx
  if (cpu_info.has_avx512_f())
    return "avx512";
  if (cpu_info.has_avx2())
    return "avx2";
  if (cpu_info.has_avx())
    return "avx";
  return "noavx";
}

inline std::string GetSuitableCudaVariant(
    const std::vector<std::string>& variants, const std::string& cuda_version) {
  std::regex cuda_reg("cuda-(\\d+)-(\\d+)");
  std::smatch match;

  int requestedMajor = 0;
  int requestedMinor = 0;

  if (!cuda_version.empty()) {
    // Split the provided CUDA version into major and minor parts
    sscanf(cuda_version.c_str(), "%d.%d", &requestedMajor, &requestedMinor);
  }

  std::string selectedVariant;
  int bestMatchMajor = -1;
  int bestMatchMinor = -1;

  for (const auto& variant : variants) {
    if (std::regex_search(variant, match, cuda_reg)) {
      // Found a CUDA version in the variant
      int variantMajor = std::stoi(match[1]);
      int variantMinor = std::stoi(match[2]);

      if (requestedMajor == variantMajor) {
        // If the major versions match, prefer the closest minor version
        if (requestedMinor >= variantMinor &&
            (variantMajor > bestMatchMajor ||
             (variantMajor == bestMatchMajor &&
              variantMinor > bestMatchMinor))) {
          selectedVariant = variant;
          bestMatchMajor = variantMajor;
          bestMatchMinor = variantMinor;
        }
      }
    }
  }

  // If no CUDA version is provided, select the variant without any CUDA in the name
  if (selectedVariant.empty()) {
    LOG_WARN
        << "No suitable CUDA variant found, selecting a variant without CUDA";
    for (const auto& variant : variants) {
      if (variant.find("cuda") == std::string::npos) {
        selectedVariant = variant;
        LOG_INFO << "Found variant without CUDA: " << selectedVariant << "\n";
        break;
      }
    }
  }

  return selectedVariant;
}

inline std::string ValidateTensorrtLlm(const std::vector<std::string>& variants,
                                       const std::string& os,
                                       const std::string& cuda_version) {
  std::vector<std::string> os_compatible_list;
  std::copy_if(variants.begin(), variants.end(),
               std::back_inserter(os_compatible_list),
               [&os](const std::string& variant) {
                 auto os_match = "-" + os;
                 return variant.find(os_match) != std::string::npos;
               });
  auto cuda_compatible =
      GetSuitableCudaVariant(os_compatible_list, cuda_version);
  return cuda_compatible;
}

inline std::string ValidateOnnx(const std::vector<std::string>& variants,
                                const std::string& os,
                                const std::string& cpu_arch) {

  std::vector<std::string> os_and_arch_compatible_list;
  std::copy_if(variants.begin(), variants.end(),
               std::back_inserter(os_and_arch_compatible_list),
               [&os, &cpu_arch](const std::string& variant) {
                 auto os_match = "-" + os;
                 auto cpu_arch_match = "-" + cpu_arch;

                 return variant.find(os_match) != std::string::npos &&
                        variant.find(cpu_arch_match) != std::string::npos;
               });
  if (!os_and_arch_compatible_list.empty())
    return os_and_arch_compatible_list[0];
  return "";
}

inline std::string Validate(const std::vector<std::string>& variants,
                            const std::string& os, const std::string& cpu_arch,
                            const std::string& suitable_avx,
                            const std::string& cuda_version, bool cpu_only) {
  // Early return if the OS is not supported
  if (os != "mac" && os != "windows" && os != "linux") {
    return "";
  }

  std::vector<std::string> os_and_arch_compatible_list;
  std::copy_if(variants.begin(), variants.end(),
               std::back_inserter(os_and_arch_compatible_list),
               [&os, &cpu_arch](const std::string& variant) {
                 auto os_match = "-" + os;
                 auto cpu_arch_match = "-" + cpu_arch;

                 return variant.find(os_match) != std::string::npos &&
                        variant.find(cpu_arch_match) != std::string::npos;
               });

  if (os == "mac" && !os_and_arch_compatible_list.empty())
    return os_and_arch_compatible_list[0];

  std::vector<std::string> avx_compatible_list;

  std::copy_if(os_and_arch_compatible_list.begin(),
               os_and_arch_compatible_list.end(),
               std::back_inserter(avx_compatible_list),
               [&suitable_avx, &os](const std::string& variant) {
                 auto os_match = "-" + os;
                 auto suitable_avx_match = "-" + suitable_avx;

                 return variant.find(os_match) != std::string::npos &&
                        variant.find(suitable_avx_match) != std::string::npos;
               });

  if (cpu_only) {
    std::string selected_variant;
    LOG_INFO << "CPU mode only, selecting a variant without CUDA";
    for (const auto& variant : avx_compatible_list) {
      if (variant.find("cuda") == std::string::npos) {
        selected_variant = variant;
        LOG_INFO << "Found variant without CUDA: " << selected_variant << "\n";
        break;
      }
    }
    return selected_variant;
  }

  auto cuda_compatible =
      GetSuitableCudaVariant(avx_compatible_list, cuda_version);

  return cuda_compatible;
}
}  // namespace engine_matcher_utils
