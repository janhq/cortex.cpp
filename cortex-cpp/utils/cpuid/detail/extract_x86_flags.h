#pragma once

#include <cstdint>

#include "cpu_info_impl.h"

namespace cortex::cpuid {

void extract_x86_flags(CpuInfo::Impl& info, uint32_t ecx, uint32_t edx) {
  // Instruction set flags

  info.has_fpu = (edx & (1 << 0)) != 0;
  info.has_mmx = (edx & (1 << 23)) != 0;
  info.has_sse = (edx & (1 << 25)) != 0;
  info.has_sse2 = (edx & (1 << 26)) != 0;
  info.has_sse3 = (ecx & (1 << 0)) != 0;
  info.has_ssse3 = (ecx & (1 << 9)) != 0;
  info.has_sse4_1 = (ecx & (1 << 19)) != 0;
  info.has_sse4_2 = (ecx & (1 << 20)) != 0;
  info.has_pclmulqdq = (ecx & (1 << 1)) != 0;
  info.has_avx = (ecx & (1 << 28)) != 0;
  info.has_aes = (ecx & (1 << 25)) != 0;
  info.has_f16c = (ecx & (1 << 29)) != 0;
}

void extract_x86_extended_flags(CpuInfo::Impl& info, uint32_t ebx, uint32_t ecx,
                                uint32_t edx) {
  // Extended instruction set flags

  info.has_avx2 = (ebx & (1 << 5)) != 0;
  info.has_avx512_f = (ebx & (1 << 16)) != 0;
  info.has_avx512_dq = (ebx & (1 << 17)) != 0;
  info.has_avx512_ifma = (ebx & (1 << 21)) != 0;
  info.has_avx512_pf = (ebx & (1 << 26)) != 0;
  info.has_avx512_er = (ebx & (1 << 27)) != 0;
  info.has_avx512_cd = (ebx & (1 << 28)) != 0;
  info.has_avx512_bw = (ebx & (1 << 30)) != 0;
  info.has_avx512_vl = (ebx & (1 << 31)) != 0;
  info.has_avx512_vbmi = (ecx & (1 << 1)) != 0;
  info.has_avx512_vbmi2 = (ecx & (1 << 6)) != 0;
  info.has_avx512_vnni = (ecx & (1 << 11)) != 0;
  info.has_avx512_bitalg = (ecx & (1 << 12)) != 0;
  info.has_avx512_vpopcntdq = (ecx & (1 << 14)) != 0;
  info.has_avx512_4vnniw = (edx & (1 << 2)) != 0;
  info.has_avx512_4fmaps = (edx & (1 << 3)) != 0;
  info.has_avx512_vp2intersect = (edx & (1 << 8)) != 0;
}
}  // namespace cortex::cpuid