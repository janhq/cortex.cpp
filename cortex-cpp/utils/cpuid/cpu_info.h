// Copyright (c) 2013 Steinwurf ApS
// All Rights Reserved
// Inspired by https://github.com/steinwurf/cpuid
#pragma once

#include <memory>
#include <string>

namespace cortex::cpuid {
/// The CpuInfo object extract information about which, if any, additional
/// instructions are supported by the CPU.
class CpuInfo {
 public:
  /// Constructor for feature detection with default values
  CpuInfo();

  /// Destructor
  ~CpuInfo();

  /// Return true if the CPU supports x87 Floating-Point Unit
  bool has_fpu() const;

  /// Return true if the CPU supports MMX
  bool has_mmx() const;

  /// Return true if the CPU supports Streaming SIMD Extensions
  bool has_sse() const;

  /// Return true if the CPU supports Streaming SIMD Extensions 2
  bool has_sse2() const;

  /// Return true if the CPU supports Streaming SIMD Extensions 3
  bool has_sse3() const;

  /// Return true if the CPU supports Supplemental Streaming SIMD Extensions 3
  bool has_ssse3() const;

  /// Return true if the CPU supports Streaming SIMD Extensions 4.1
  bool has_sse4_1() const;

  /// Return true if the CPU supports Streaming SIMD Extensions 4.2
  bool has_sse4_2() const;

  /// Return true if the CPU supports carry-less multiplication of two 64-bit
  /// polynomials over the finite field GF(2)
  bool has_pclmulqdq() const;

  /// Return true if the CPU supports Advanced Vector Extensions
  bool has_avx() const;

  /// Return true if the CPU supports AVX Vector Neural Network
  /// Instructions
  bool has_avx_vnni() const;

  /// Return true if the CPU supports Advanced Vector Extensions 2
  bool has_avx2() const;

  /// Return true if the CPU supports AVX-512 Foundation
  bool has_avx512_f() const;

  /// Return true if the CPU supports AVX-512 Doubleword and Quadword
  /// Instructions
  bool has_avx512_dq() const;

  /// Return true if the CPU supports AVX-512 Integer Fused Multiply Add
  bool has_avx512_ifma() const;

  /// Return true if the CPU supports AVX-512 Prefetch Instructions
  bool has_avx512_pf() const;

  /// Return true if the CPU supports AVX-512 Exponential and Reciprocal
  /// Instructions
  bool has_avx512_er() const;

  /// Return true if the CPU supports AVX-512 Conflict Detection Instructions
  bool has_avx512_cd() const;

  /// Return true if the CPU supports AVX-512 Byte and Word Instructions
  bool has_avx512_bw() const;

  /// Return true if the CPU supports AVX-512 Vector Length Extensions
  bool has_avx512_vl() const;

  /// Return true if the CPU supports AVX-512 Vector Byte Manipulation
  /// Instructions
  bool has_avx512_vbmi() const;

  /// Return true if the CPU supports AVX-512 Vector Byte Manipulation
  /// Instructions 2
  bool has_avx512_vbmi2() const;

  /// Return true if the CPU supports AVX-512 Vector Neural Network
  /// Instructions
  bool has_avx512_vnni() const;

  /// Return true if the CPU supports AVX-512 Bit Algorithms
  bool has_avx512_bitalg() const;

  /// Return true if the CPU supports Vector population count instruction
  bool has_avx512_vpopcntdq() const;

  /// Return true if the CPU supports AVX-512 Vector Neural Network
  /// Instructions Word variable precision
  bool has_avx512_4vnniw() const;

  /// Return true if the CPU supports AVX-512 Fused Multiply Accumulation
  /// Packed Single precision
  bool has_avx512_4fmaps() const;

  /// Return true if the CPU supports AVX-512 Vector Pair Intersection to a
  /// Pair of Mask Registers
  bool has_avx512_vp2intersect() const;

  /// Return true if the CPU supports converting between half-precision and
  /// standard IEEE single-precision floating-point formats
  bool has_f16c() const;

  /// Return true if the CPU supports Advanced Encryption Standard instruction
  /// set
  bool has_aes() const;

  /// Return true if the CPU supports ARM Advanced SIMD
  bool has_neon() const;

  std::string to_string();

 public:
  /// Private implementation
  struct Impl;

 private:
  /// Pimpl pointer
  std::unique_ptr<Impl> impl;
};
}  // namespace cortex::cpuid