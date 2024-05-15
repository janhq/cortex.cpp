#pragma once

#include <cstdint>

#include "cpu_info_impl.h"
#include "extract_x86_flags.h"

namespace cortex::cpuid {

// Reference for this code is Intel's recommendation for detecting AVX2
// on Haswell located here: http://goo.gl/c6IkGX
void run_cpuid(uint32_t eax, uint32_t ecx, uint32_t* abcd) {
  uint32_t ebx = 0, edx = 0;

#if defined(__i386__) && defined(__PIC__)
  // If PIC used under 32-bit, EBX cannot be clobbered
  // EBX is saved to EDI and later restored
  __asm__(
      "movl %%ebx, %%edi;"
      "cpuid;"
      "xchgl %%ebx, %%edi;"
      : "=D"(ebx), "+a"(eax), "+c"(ecx), "=d"(edx));
#else
  __asm__("cpuid;" : "+b"(ebx), "+a"(eax), "+c"(ecx), "=d"(edx));
#endif

  abcd[0] = eax;
  abcd[1] = ebx;
  abcd[2] = ecx;
  abcd[3] = edx;
}

/// @todo Document
void init_cpuinfo(CpuInfo::Impl& info) {
  // Note: We need to capture these 4 registers, otherwise we get
  // a segmentation fault on 32-bit Linux
  uint32_t output[4];

  // The register information per input can be extracted from here:
  // http://en.wikipedia.org/wiki/CPUID

  // CPUID should be called with EAX=0 first, as this will return the
  // maximum supported EAX input value for future calls
  run_cpuid(0, 0, output);
  uint32_t maximum_index = output[0];

  // Set registers for basic flag extraction
  // All CPUs should support index=1
  if (maximum_index >= 1U) {
    run_cpuid(1, 0, output);
    extract_x86_flags(info, output[2], output[3]);
  }

  // Set registers for extended flags extraction using index=7
  // This operation is not supported on older CPUs, so it should be skipped
  // to avoid incorrect results
  if (maximum_index >= 7U) {
    run_cpuid(7, 0, output);
    extract_x86_extended_flags(info, output[1], output[2], output[3]);
  }
}
}  // namespace cortex::cpuid