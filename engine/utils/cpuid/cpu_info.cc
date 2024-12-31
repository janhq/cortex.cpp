// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
// Inspired by https://github.com/steinwurf/cpuid
#include "platform.h"

#include "cpu_info.h"
#include "detail/cpu_info_impl.h"

#if defined(PLATFORM_GCC_COMPATIBLE_X86)
#include "detail/init_gcc_x86.h"
#elif defined(PLATFORM_MSVC_X86) && !defined(PLATFORM_WINDOWS_PHONE)
#include "detail/init_msvc_x86.h"
#elif defined(PLATFORM_MSVC_ARM)
#include "detail/init_msvc_arm.h"
#elif defined(PLATFORM_CLANG_ARM) && defined(PLATFORM_IOS)
#include "detail/init_ios_clang_arm.h"
#elif defined(PLATFORM_GCC_COMPATIBLE_ARM) && defined(PLATFORM_LINUX)
#include "detail/init_linux_gcc_arm.h"
#else
#include "detail/init_unknown.h"
#endif

namespace cortex::cpuid {

CpuInfo::CpuInfo() : impl(new Impl()) {
  init_cpuinfo(*impl);
}

CpuInfo::~CpuInfo() = default;

// x86 member functions
bool CpuInfo::has_fpu() const {
  return impl->has_fpu;
}

bool CpuInfo::has_mmx() const {
  return impl->has_mmx;
}

bool CpuInfo::has_sse() const {
  return impl->has_sse;
}

bool CpuInfo::has_sse2() const {
  return impl->has_sse2;
}

bool CpuInfo::has_sse3() const {
  return impl->has_sse3;
}

bool CpuInfo::has_ssse3() const {
  return impl->has_ssse3;
}

bool CpuInfo::has_sse4_1() const {
  return impl->has_sse4_1;
}

bool CpuInfo::has_sse4_2() const {
  return impl->has_sse4_2;
}

bool CpuInfo::has_pclmulqdq() const {
  return impl->has_pclmulqdq;
}

bool CpuInfo::has_avx() const {
  return impl->has_avx;
}

bool CpuInfo::has_avx2() const {
  return impl->has_avx2;
}

bool CpuInfo::has_avx512_f() const {
  return impl->has_avx512_f;
}

bool CpuInfo::has_avx512_dq() const {
  return impl->has_avx512_dq;
}

bool CpuInfo::has_avx512_ifma() const {
  return impl->has_avx512_ifma;
}

bool CpuInfo::has_avx512_pf() const {
  return impl->has_avx512_pf;
}

bool CpuInfo::has_avx512_er() const {
  return impl->has_avx512_er;
}

bool CpuInfo::has_avx512_cd() const {
  return impl->has_avx512_cd;
}

bool CpuInfo::has_avx512_bw() const {
  return impl->has_avx512_bw;
}

bool CpuInfo::has_avx512_vl() const {
  return impl->has_avx512_vl;
}

bool CpuInfo::has_avx512_vbmi() const {
  return impl->has_avx512_vbmi;
}

bool CpuInfo::has_avx512_vbmi2() const {
  return impl->has_avx512_vbmi2;
}

bool CpuInfo::has_avx512_vnni() const {
  return impl->has_avx512_vnni;
}

bool CpuInfo::has_avx512_bitalg() const {
  return impl->has_avx512_bitalg;
}

bool CpuInfo::has_avx512_vpopcntdq() const {
  return impl->has_avx512_vpopcntdq;
}

bool CpuInfo::has_avx512_4vnniw() const {
  return impl->has_avx512_4vnniw;
}

bool CpuInfo::has_avx512_4fmaps() const {
  return impl->has_avx512_4fmaps;
}

bool CpuInfo::has_avx512_vp2intersect() const {
  return impl->has_avx512_vp2intersect;
}

bool CpuInfo::has_f16c() const {
  return impl->has_f16c;
}

bool CpuInfo::has_aes() const {
  return impl->has_aes;
}

// ARM member functions
bool CpuInfo::has_neon() const {
  return impl->has_neon;
}

std::string CpuInfo::to_string() {
  std::string s;
  auto get = [](bool flag) -> std::string {
    return flag ? "1" : "0";
  };
  s += "fpu = " + get(impl->has_fpu) + "| ";
  s += "mmx = " + get(impl->has_mmx) + "| ";
  s += "sse = " + get(impl->has_sse) + "| ";
  s += "sse2 = " + get(impl->has_sse2) + "| ";
  s += "sse3 = " + get(impl->has_sse3) + "| ";
  s += "ssse3 = " + get(impl->has_ssse3) + "| ";
  s += "sse4_1 = " + get(impl->has_sse4_1) + "| ";
  s += "sse4_2 = " + get(impl->has_sse4_2) + "| ";
  s += "pclmulqdq = " + get(impl->has_pclmulqdq) + "| ";
  s += "avx = " + get(impl->has_avx) + "| ";
  s += "avx2 = " + get(impl->has_avx2) + "| ";
  s += "avx512_f = " + get(impl->has_avx512_f) + "| ";
  s += "avx512_dq = " + get(impl->has_avx512_dq) + "| ";
  s += "avx512_ifma = " + get(impl->has_avx512_ifma) + "| ";
  s += "avx512_pf = " + get(impl->has_avx512_pf) + "| ";
  s += "avx512_er = " + get(impl->has_avx512_er) + "| ";
  s += "avx512_cd = " + get(impl->has_avx512_cd) + "| ";
  s += "avx512_bw = " + get(impl->has_avx512_bw) + "| ";
  s += "avx512_vl = " + get(impl->has_avx512_vl) + "| ";
  s += "avx512_vbmi = " + get(impl->has_avx512_vbmi) + "| ";
  s += "avx512_vbmi2 = " + get(impl->has_avx512_vbmi2) + "| ";
  s += "avx512_vnni = " + get(impl->has_avx512_vnni) + "| ";
  s += "avx512_bitalg = " + get(impl->has_avx512_bitalg) + "| ";
  s += "avx512_vpopcntdq = " + get(impl->has_avx512_vpopcntdq) + "| ";
  s += "avx512_4vnniw = " + get(impl->has_avx512_4vnniw) + "| ";
  s += "avx512_4fmaps = " + get(impl->has_avx512_4fmaps) + "| ";
  s += "avx512_vp2intersect = " + get(impl->has_avx512_vp2intersect) + "| ";
  s += "aes = " + get(impl->has_aes) + "| ";
  s += "f16c = " + get(impl->has_f16c) + "|";
  return s;
}

std::vector<std::string> CpuInfo::instructions() {
  std::vector<std::string> res;
#define ADD_FEATURE_IF_PRESENT(feature_name) \
  if (impl->has_##feature_name)              \
    res.emplace_back(#feature_name);

  ADD_FEATURE_IF_PRESENT(fpu);
  ADD_FEATURE_IF_PRESENT(mmx);
  ADD_FEATURE_IF_PRESENT(sse);
  ADD_FEATURE_IF_PRESENT(sse2);
  ADD_FEATURE_IF_PRESENT(sse3);
  ADD_FEATURE_IF_PRESENT(ssse3);
  ADD_FEATURE_IF_PRESENT(sse4_1);
  ADD_FEATURE_IF_PRESENT(sse4_2);
  ADD_FEATURE_IF_PRESENT(pclmulqdq);
  ADD_FEATURE_IF_PRESENT(avx);
  ADD_FEATURE_IF_PRESENT(avx2);
  ADD_FEATURE_IF_PRESENT(avx512_f);
  ADD_FEATURE_IF_PRESENT(avx512_dq);
  ADD_FEATURE_IF_PRESENT(avx512_ifma);
  ADD_FEATURE_IF_PRESENT(avx512_pf);
  ADD_FEATURE_IF_PRESENT(avx512_er);
  ADD_FEATURE_IF_PRESENT(avx512_cd);
  ADD_FEATURE_IF_PRESENT(avx512_bw);
  ADD_FEATURE_IF_PRESENT(avx512_vl);
  ADD_FEATURE_IF_PRESENT(avx512_vbmi);
  ADD_FEATURE_IF_PRESENT(avx512_vbmi2);
  ADD_FEATURE_IF_PRESENT(avx512_vnni);
  ADD_FEATURE_IF_PRESENT(avx512_bitalg);
  ADD_FEATURE_IF_PRESENT(avx512_vpopcntdq);
  ADD_FEATURE_IF_PRESENT(avx512_4vnniw);
  ADD_FEATURE_IF_PRESENT(avx512_4fmaps);
  ADD_FEATURE_IF_PRESENT(avx512_vp2intersect);
  ADD_FEATURE_IF_PRESENT(aes);
  ADD_FEATURE_IF_PRESENT(f16c);
#undef ADD_FEATURE_IF_PRESENT
  return res;
}

}  // namespace cortex::cpuid
