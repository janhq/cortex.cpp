#include "cpu_validation.h"
#include "cpu_info.h"

namespace cortex::cpuid::llamacpp {

// TODO implement Result for better perf
std::pair<bool, std::string> IsValidInstructions() {
  cpuid::CpuInfo info;
#if defined(_WIN32)
#if defined(CORTEX_AVX512)
  auto res = info.has_avx512_f() || info.has_avx512_dq() ||
             info.has_avx512_ifma() || info.has_avx512_pf() ||
             info.has_avx512_er() || info.has_avx512_cd() ||
             info.has_avx512_bw() || info.has_avx512_vl() ||
             info.has_avx512_vbmi() || info.has_avx512_vbmi2() ||
             info.has_avx512_vnni() || info.has_avx512_bitalg() ||
             info.has_avx512_vpopcntdq() || info.has_avx512_4vnniw() ||
             info.has_avx512_4fmaps() || info.has_avx512_vp2intersect();
  return res ? std::make_pair(true, "")
             : std::make_pair(false, "System does not support AVX512");
#elif defined(CORTEX_AVX2)
  return info.has_avx2()
             ? std::make_pair(true, "")
             : std::make_pair(false, "System does not support AVX2");
#elif defined(CORTEX_VULKAN)
  return std::make_pair(true, "");
#else
  return info.has_avx() ? std::make_pair(true, "")
                        : std::make_pair(false, "System does not support AVX");
#endif
#elif defined(__APPLE__)
  return std::make_pair(true, "");
#else
#if defined(CORTEX_CUDA)
  return std::make_pair(true, "");
#elif defined(CORTEX_AVX512)
  auto res = info.has_avx512_f() || info.has_avx512_dq() ||
             info.has_avx512_ifma() || info.has_avx512_pf() ||
             info.has_avx512_er() || info.has_avx512_cd() ||
             info.has_avx512_bw() || info.has_avx512_vl() ||
             info.has_avx512_vbmi() || info.has_avx512_vbmi2() ||
             info.has_avx512_vnni() || info.has_avx512_bitalg() ||
             info.has_avx512_vpopcntdq() || info.has_avx512_4vnniw() ||
             info.has_avx512_4fmaps() || info.has_avx512_vp2intersect();
  return res ? std::make_pair(true, "")
             : std::make_pair(false, "System does not support AVX512");
#elif defined(CORTEX_AVX2)
  return info.has_avx2()
             ? std::make_pair(true, "")
             : std::make_pair(false, "System does not support AVX2");
#elif defined(CORTEX_VULKAN)
  return std::make_pair(true, "");
#else
  return info.has_avx() ? std::make_pair(true, "")
                        : std::make_pair(false, "System does not support AVX");
#endif
#endif
  return std::make_pair(true, "");
}
}  // namespace cortex::cpuid::llamacpp