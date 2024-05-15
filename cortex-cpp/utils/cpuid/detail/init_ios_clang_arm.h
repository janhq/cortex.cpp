#pragma once

#include "cpu_info_impl.h"

namespace cortex::cpuid {

void init_cpuinfo(CpuInfo::Impl& info) {
  // The __ARM_NEON__ macro will be defined by the Apple Clang compiler
  // when targeting ARMv7 processors that have NEON.
  // The compiler guarantees this capability, so there is no benefit
  // in doing a runtime check. More info in this SO answer:
  // http://stackoverflow.com/a/1601234

#if defined __ARM_NEON__
  info.has_neon = true;
#else
  info.has_neon = false;
#endif
}
}  // namespace cortex::cpuid