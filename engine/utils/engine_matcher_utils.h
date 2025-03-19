#pragma once

#include <trantor/utils/Logger.h>
#include <algorithm>
#include <iterator>
#include <regex>
#include <string>
#include <vector>
#include "utils/cpuid/cpu_info.h"
#include "utils/engine_constants.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"
#include "utils/string_utils.h"

namespace engine_matcher_utils {
/**
 * Extracting variant and version info from file name.
 */
inline cpp::result<std::string, std::string> GetVariantFromNameAndVersion(
    const std::string& engine_file_name, const std::string& engine,
    const std::string& version) {
  if (engine_file_name.empty()) {
    return cpp::fail("Engine file name is empty");
  }
  if (engine.empty()) {
    return cpp::fail("Engine name is empty");
  }
  CTL_DBG("version: " << version);
  namespace su = string_utils;
  CTL_DBG("engine_file_name: " << engine_file_name);
  auto rm_extension_menlo = su::RemoveSubstring(engine_file_name, ".tar.gz");
  auto rm_extension_ggml = su::RemoveSubstring(rm_extension_menlo, ".zip");
  CTL_DBG("removed_extension: " << rm_extension_ggml);
  auto version_and_variant =
      su::RemoveSubstring(rm_extension_ggml, engine + "-");
  CTL_DBG("version_and_variant: " << version_and_variant);
  auto variant = su::RemoveSubstring(version_and_variant, version + "-");
  auto v = su::RemoveSubstring(variant, "llama-bin-");
  CTL_DBG("variant: " << v);
  return v;
}

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

  int requested_major = 0;
  int requested_minor = 0;

  if (!cuda_version.empty()) {
// Split the provided CUDA version into major and minor parts
#if defined(_MSC_VER)
    sscanf_s(cuda_version.c_str(), "%d.%d", &requested_major, &requested_minor);
#else
    sscanf(cuda_version.c_str(), "%d.%d", &requested_major, &requested_minor);
#endif
  }

  std::string selected_variant;
  int best_match_major = -1;
  int best_match_minor = -1;

  for (const auto& variant : variants) {
    if (std::regex_search(variant, match, cuda_reg)) {
      // Found a CUDA version in the variant
      int variant_major = std::stoi(match[1]);
      int variant_minor = std::stoi(match[2]);

      if (requested_major == variant_major) {
        // If the major versions match, prefer the closest minor version
        if (requested_minor >= variant_minor &&
            (variant_major > best_match_major ||
             (variant_major == best_match_major &&
              variant_minor > best_match_minor))) {
          selected_variant = variant;
          best_match_major = variant_major;
          best_match_minor = variant_minor;
        }
      }
    }
  }

  // If no CUDA version is provided, select the variant without any CUDA in the name
  if (selected_variant.empty()) {
    LOG_WARN
        << "No suitable CUDA variant found, selecting a variant without CUDA";
    for (const auto& variant : variants) {
      if (variant.find("cuda") == std::string::npos) {
        selected_variant = variant;
        LOG_INFO << "Found variant without CUDA: " << selected_variant << "\n";
        break;
      }
    }
  }

  return selected_variant;
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
  CTL_INF(os << " " << cpu_arch);
  // Early return if the OS is not supported
  if (os != kMacOs && os != kWindowsOs && os != kLinuxOs) {
    return "";
  }

  std::vector<std::string> os_and_arch_compatible_list;
  std::copy_if(variants.begin(), variants.end(),
               std::back_inserter(os_and_arch_compatible_list),
               [&os, &cpu_arch](const std::string& variant) {
                 // In case of Linux, we need to include ubuntu version also
                 if (os == kLinuxOs) {
                   if (variant.find(kUbuntuOs) != std::string::npos &&
                       variant.find(cpu_arch) != std::string::npos)
                     return true;
                 }
                 auto os_match = "-" + os;
                 auto cpu_arch_match = "-" + cpu_arch;

                 return variant.find(os_match) != std::string::npos &&
                        variant.find(cpu_arch_match) != std::string::npos;
               });

  if (os == kMacOs && !os_and_arch_compatible_list.empty())
    return os_and_arch_compatible_list[0];

  if (os == kLinuxOs && cpu_arch == "arm64" &&
      !os_and_arch_compatible_list.empty()) {
    return os_and_arch_compatible_list[0];
  }

  std::vector<std::string> avx_compatible_list;

  std::copy_if(os_and_arch_compatible_list.begin(),
               os_and_arch_compatible_list.end(),
               std::back_inserter(avx_compatible_list),
               [&os, &cpu_arch, &suitable_avx](const std::string& variant) {
                 if (os == kLinuxOs &&
                     (suitable_avx == "avx2" || suitable_avx == "avx512" ||
                      cpu_arch == "arm64")) {
                   if (variant.find(std::string(kUbuntuOs) + "-" + cpu_arch) !=
                       std::string::npos)
                     return true;
                 }
                 auto suitable_avx_match = "-" + suitable_avx;

                 return variant.find(suitable_avx_match) != std::string::npos;
               });

  auto cuda_compatible =
      GetSuitableCudaVariant(avx_compatible_list, cuda_version);

  return cuda_compatible;
}

inline std::pair<std::string, std::string> GetVersionAndArch(
    const std::string& file_name) {
  // Remove the file extension
  std::string b = string_utils::RemoveSubstring(file_name, ".tar.gz");
  std::string base = string_utils::RemoveSubstring(b, ".zip");

  size_t arch_pos = 0;
  if (base.find("win") != std::string::npos) {
    arch_pos = base.find("-bin-win");
  } else if (base.find("linux") != std::string::npos) {
    arch_pos = base.find("-bin-linux");
  } else if (base.find("ubuntu") != std::string::npos) {
    arch_pos = base.find("-bin-ubuntu");
  } else {
    arch_pos = base.find("-bin-macos");
  }

  // Extract architecture part
  auto arch = base.substr(arch_pos + 1);

  // Extract version part
  size_t v_pos = base.find_first_of('-');
  auto version = base.substr(v_pos + 1, arch_pos - v_pos - 1);
  return std::pair(version, string_utils::RemoveSubstring(arch, "bin-"));
}
}  // namespace engine_matcher_utils
