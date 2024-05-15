#pragma once

#include "cpu_info_impl.h"

namespace cortex::cpuid {
void init_cpuinfo(CpuInfo::Impl& info) {
  // Visual Studio 2012 (and above) guarantees the NEON capability when
  // compiling for Windows Phone 8 (and above)

#if defined(PLATFORM_WINDOWS_PHONE)
  info.has_neon = true;
#else
  info.has_neon = false;
#endif
}
}  // namespace cortex::cpuid