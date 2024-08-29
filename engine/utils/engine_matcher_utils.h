#include <algorithm>
#include <iostream>
#include <iterator>
#include <regex>
#include <string>
#include <vector>
#include "utils/cpuid/cpu_info.h"
#include "utils/logging_utils.h"

namespace engine_matcher_utils {
// for testing purpose
const std::vector<std::string> cortex_llamacpp_variants{
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx-cuda-11-7.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx-cuda-12-0.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx2-cuda-11-7.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx2-cuda-12-0.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx2.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx512-cuda-11-7.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx512-cuda-12-0.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx512.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-noavx-cuda-11-7.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-noavx-cuda-12-0.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-noavx.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-vulkan.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-mac-amd64.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-mac-arm64.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx-cuda-11-7.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx-cuda-12-0.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx2-cuda-11-7.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx2-cuda-12-0.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx2.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx512-cuda-11-7.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx512-cuda-12-0.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx512.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-noavx-cuda-11-7.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-noavx-cuda-12-0.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-noavx.tar.gz",
    "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-vulkan.tar.gz",
};
const std::vector<std::string> cortex_onnx_variants{
    "cortex.onnx-0.1.7-windows-amd64.tar.gz"};

const std::vector<std::string> cortex_tensorrt_variants{
    "cortex.tensorrt-llm-0.0.9-linux-cuda-12-4.tar.gz",
    "cortex.tensorrt-llm-0.0.9-windows-cuda-12-4.tar.gz"};

inline std::string GetSuitableAvxVariant() {
  cortex::cpuid::CpuInfo cpu_info;

  CTLOG_INFO("GetSuitableAvxVariant:" << "\n" << cpu_info.to_string());

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
    } else if (cuda_version.empty() && selectedVariant.empty()) {
      // If no CUDA version is provided, select the variant without any CUDA in the name
      selectedVariant = variant;
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
                            const std::string& cuda_version) {

  // Early return if the OS is unsupported
  if (os != "mac" && os != "windows" && os != "linux") {
    // TODO: throw is better
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
               [&suitable_avx](const std::string& variant) {
                 auto suitable_avx_match = "-" + suitable_avx;

                 return variant.find(suitable_avx_match) != std::string::npos;
               });

  auto cuda_compatible =
      GetSuitableCudaVariant(avx_compatible_list, cuda_version);

  return cuda_compatible;
}
}  // namespace engine_matcher_utils