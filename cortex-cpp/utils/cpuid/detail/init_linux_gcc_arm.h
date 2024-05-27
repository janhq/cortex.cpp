// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
// Inspired by https://github.com/steinwurf/cpuid
#pragma once

#include <cassert>
#include <cstdio>
#include <cstring>

#include <elf.h>
#include <fcntl.h>
#include <linux/auxvec.h>
#include <unistd.h>

#include "cpu_info_impl.h"

namespace cortex::cpuid {

void init_cpuinfo(CpuInfo::Impl& info) {
#if defined(__aarch64__)
  // The Advanced SIMD (NEON) instruction set is required on AArch64
  // (64-bit ARM). Note that /proc/cpuinfo will display "asimd" instead of
  // "neon" in the Features list on a 64-bit ARM CPU.
  info.m_has_neon = true;
#else
  // Runtime detection of NEON is necessary on 32-bit ARM CPUs
  //
  // Follow recommendation from Cortex-A Series Programmer's guide
  // in Section 20.1.7 Detecting NEON. The guide is available at
  // Steinwurf's Google drive: steinwurf/technical/experimental/cpuid

  auto cpufile = open("/proc/self/auxv", O_RDONLY);
  assert(cpufile);

  Elf32_auxv_t auxv;

  if (cpufile >= 0) {
    const auto size_auxv_t = sizeof(Elf32_auxv_t);
    while (read(cpufile, &auxv, size_auxv_t) == size_auxv_t) {
      if (auxv.a_type == AT_HWCAP) {
        info.has_neon = (auxv.a_un.a_val & 4096) != 0;
        break;
      }
    }

    close(cpufile);
  } else {
    info.has_neon = false;
  }
#endif
}
}  // namespace cortex::cpuid