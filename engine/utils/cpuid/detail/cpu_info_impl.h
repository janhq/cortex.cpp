// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
// Inspired by https://github.com/steinwurf/cpuid
#pragma once

#include "../cpu_info.h"

namespace cortex::cpuid {

struct CpuInfo::Impl {
  Impl()
      : has_fpu(false),
        has_mmx(false),
        has_sse(false),
        has_sse2(false),
        has_sse3(false),
        has_ssse3(false),
        has_sse4_1(false),
        has_sse4_2(false),
        has_pclmulqdq(false),
        has_avx(false),
        has_avx2(false),
        has_avx512_f(false),
        has_avx512_dq(false),
        has_avx512_ifma(false),
        has_avx512_pf(false),
        has_avx512_er(false),
        has_avx512_cd(false),
        has_avx512_bw(false),
        has_avx512_vl(false),
        has_avx512_vbmi(false),
        has_avx512_vbmi2(false),
        has_avx512_vnni(false),
        has_avx512_bitalg(false),
        has_avx512_vpopcntdq(false),
        has_avx512_4vnniw(false),
        has_avx512_4fmaps(false),
        has_avx512_vp2intersect(false),
        has_f16c(false),
        has_aes(false),
        has_neon(false) {}

  bool has_fpu;
  bool has_mmx;
  bool has_sse;
  bool has_sse2;
  bool has_sse3;
  bool has_ssse3;
  bool has_sse4_1;
  bool has_sse4_2;
  bool has_pclmulqdq;
  bool has_avx;
  bool has_avx2;
  bool has_avx512_f;
  bool has_avx512_dq;
  bool has_avx512_ifma;
  bool has_avx512_pf;
  bool has_avx512_er;
  bool has_avx512_cd;
  bool has_avx512_bw;
  bool has_avx512_vl;
  bool has_avx512_vbmi;
  bool has_avx512_vbmi2;
  bool has_avx512_vnni;
  bool has_avx512_bitalg;
  bool has_avx512_vpopcntdq;
  bool has_avx512_4vnniw;
  bool has_avx512_4fmaps;
  bool has_avx512_vp2intersect;
  bool has_f16c;
  bool has_aes;
  bool has_neon;
};
}  // namespace cortex::cpuid