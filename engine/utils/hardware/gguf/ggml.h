#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "utils/result.hpp"

namespace hardware {
enum GGMLType {
  GGML_TYPE_F32 = 0,
  GGML_TYPE_F16 = 1,
  GGML_TYPE_Q4_0 = 2,
  GGML_TYPE_Q4_1 = 3,
  // GGML_TYPE_Q4_2 = 4, support has been removed
  // GGML_TYPE_Q4_3 = 5, support has been removed
  GGML_TYPE_Q5_0 = 6,
  GGML_TYPE_Q5_1 = 7,
  GGML_TYPE_Q8_0 = 8,
  GGML_TYPE_Q8_1 = 9,
  GGML_TYPE_Q2_K = 10,
  GGML_TYPE_Q3_K = 11,
  GGML_TYPE_Q4_K = 12,
  GGML_TYPE_Q5_K = 13,
  GGML_TYPE_Q6_K = 14,
  GGML_TYPE_Q8_K = 15,
  GGML_TYPE_IQ2_XXS = 16,
  GGML_TYPE_IQ2_XS = 17,
  GGML_TYPE_IQ3_XXS = 18,
  GGML_TYPE_IQ1_S = 19,
  GGML_TYPE_IQ4_NL = 20,
  GGML_TYPE_IQ3_S = 21,
  GGML_TYPE_IQ2_S = 22,
  GGML_TYPE_IQ4_XS = 23,
  GGML_TYPE_I8 = 24,
  GGML_TYPE_I16 = 25,
  GGML_TYPE_I32 = 26,
  GGML_TYPE_I64 = 27,
  GGML_TYPE_F64 = 28,
  GGML_TYPE_IQ1_M = 29,
  GGML_TYPE_BF16 = 30,
  GGML_TYPE_Q4_0_4_4 = 31,
  GGML_TYPE_Q4_0_4_8 = 32,
  GGML_TYPE_Q4_0_8_8 = 33,
  GGML_TYPE_TQ1_0 = 34,
  GGML_TYPE_TQ2_0 = 35,
  GGML_TYPE_COUNT,
};

struct GGMLTypeTrait {
  uint64_t block_size;
  uint64_t type_size;
  bool is_quantized;
};

const std::unordered_map<GGMLType, GGMLTypeTrait> kGGMLTypeTraits = {
    {GGML_TYPE_F32, {.block_size = 1, .type_size = 4}},
    {GGML_TYPE_F16, {.block_size = 1, .type_size = 2}},
    {GGML_TYPE_Q4_0, {.block_size = 32, .type_size = 18, .is_quantized = true}},
    {GGML_TYPE_Q4_1, {.block_size = 32, .type_size = 20, .is_quantized = true}},
    {GGML_TYPE_Q5_0, {.block_size = 32, .type_size = 22, .is_quantized = true}},
    {GGML_TYPE_Q5_1, {.block_size = 32, .type_size = 24, .is_quantized = true}},
    {GGML_TYPE_Q8_0, {.block_size = 32, .type_size = 34, .is_quantized = true}},
    {GGML_TYPE_Q8_1, {.block_size = 32, .type_size = 36, .is_quantized = true}},
    {GGML_TYPE_Q2_K,
     {.block_size = 256, .type_size = 84, .is_quantized = true}},
    {GGML_TYPE_Q3_K,
     {.block_size = 256, .type_size = 110, .is_quantized = true}},
    {GGML_TYPE_Q4_K,
     {.block_size = 256, .type_size = 144, .is_quantized = true}},
    {GGML_TYPE_Q5_K,
     {.block_size = 256, .type_size = 176, .is_quantized = true}},
    {GGML_TYPE_Q6_K,
     {.block_size = 256, .type_size = 210, .is_quantized = true}},
    {GGML_TYPE_Q8_K,
     {.block_size = 256, .type_size = 292, .is_quantized = true}},
    {GGML_TYPE_IQ2_XXS,
     {.block_size = 256, .type_size = 66, .is_quantized = true}},
    {GGML_TYPE_IQ2_XS,
     {.block_size = 256, .type_size = 74, .is_quantized = true}},
    {GGML_TYPE_IQ3_XXS,
     {.block_size = 256, .type_size = 98, .is_quantized = true}},
    {GGML_TYPE_IQ1_S,
     {.block_size = 256, .type_size = 50, .is_quantized = true}},
    {GGML_TYPE_IQ4_NL,
     {.block_size = 32, .type_size = 18, .is_quantized = true}},
    {GGML_TYPE_IQ3_S,
     {.block_size = 256, .type_size = 110, .is_quantized = true}},
    {GGML_TYPE_IQ2_S,
     {.block_size = 256, .type_size = 82, .is_quantized = true}},
    {GGML_TYPE_IQ4_XS,
     {.block_size = 256, .type_size = 136, .is_quantized = true}},
    {GGML_TYPE_I8, {.block_size = 1, .type_size = 1}},
    {GGML_TYPE_I16, {.block_size = 1, .type_size = 2}},
    {GGML_TYPE_I32, {.block_size = 1, .type_size = 4}},
    {GGML_TYPE_I64, {.block_size = 1, .type_size = 8}},
    {GGML_TYPE_F64, {.block_size = 1, .type_size = 8}},
    {GGML_TYPE_IQ1_M,
     {.block_size = 256, .type_size = 56, .is_quantized = true}},
    {GGML_TYPE_BF16, {.block_size = 1, .type_size = 2}},
    {GGML_TYPE_Q4_0_4_4,
     {.block_size = 32, .type_size = 18, .is_quantized = true}},
    {GGML_TYPE_Q4_0_4_8,
     {.block_size = 32, .type_size = 18, .is_quantized = true}},
    {GGML_TYPE_Q4_0_8_8,
     {.block_size = 32, .type_size = 18, .is_quantized = true}},
    {GGML_TYPE_TQ1_0,
     {.block_size = 256, .type_size = 54, .is_quantized = true}},
    {GGML_TYPE_TQ2_0,
     {.block_size = 256, .type_size = 66, .is_quantized = true}},
};

inline cpp::result<uint64_t, std::string> RowSizeOf(
    const std::vector<uint64_t>& dimensions, GGMLType t) {
  if (dimensions.empty())
    return cpp::fail("No dimensions");
  if (kGGMLTypeTraits.find(t) == kGGMLTypeTraits.end())
    return cpp::fail("Invalid type: " + std::to_string(t));

  auto& gt = kGGMLTypeTraits.at(t);
  auto ds = gt.type_size * dimensions[0] / gt.block_size;  // Row size
  for (size_t i = 1; i < dimensions.size(); i++) {
    ds *= dimensions[i];
  }
  return ds;
}

// GGMLPadding returns the padded size of the given size according to given align,
// see https://github.com/ggerganov/ggml/blob/0cbb7c0e053f5419cfbebb46fbf4d4ed60182cf5/include/ggml/ggml.h#L255.
uint64_t GGMLPadding(uint64_t size, uint64_t align) {
  return (size + align - 1) & ~(align - 1);
}

// GGMLMemoryPadding returns the padded size of the given size according to GGML memory padding,
// see https://github.com/ggerganov/ggml/blob/0cbb7c0/include/ggml/ggml.h#L238-L243.
uint64_t GGMLMemoryPadding(uint64_t size) {
  const uint64_t align = 16;
  return GGMLPadding(size, align);
}

// GGMLTensorSize is the size of GGML tensor in bytes,
// see https://github.com/ggerganov/ggml/blob/0cbb7c0e053f5419cfbebb46fbf4d4ed60182cf5/include/ggml/ggml.h#L606.
constexpr const uint64_t kGGMLTensorSize = 368;

// GGMLObjectSize is the size of GGML object in bytes,
// see https://github.com/ggerganov/ggml/blob/0cbb7c0e053f5419cfbebb46fbf4d4ed60182cf5/include/ggml/ggml.h#L563.
constexpr const uint64_t kGGMLObjectSize = 32;

// GGMLTensorOverhead is the overhead of GGML tensor in bytes,
// see https://github.com/ggerganov/ggml/blob/0cbb7c0e053f5419cfbebb46fbf4d4ed60182cf5/src/ggml.c#L2765-L2767.
constexpr uint64_t GGMLTensorOverhead() {
  return kGGMLTensorSize + kGGMLObjectSize;
}

// GGMLComputationGraphSize is the size of GGML computation graph in bytes.
constexpr const uint64_t kGGMLComputationGraphSize = 80;

// GGMLComputationGraphNodesMaximum is the maximum nodes of the computation graph,
// see https://github.com/ggerganov/llama.cpp/blob/7672adeec7a79ea271058c63106c142ba84f951a/llama.cpp#L103.
constexpr const uint64_t kGGMLComputationGraphNodesMaximum = 8192;

// GGMLComputationGraphNodesDefault is the default nodes of the computation graph,
// see https://github.com/ggerganov/ggml/blob/0cbb7c0e053f5419cfbebb46fbf4d4ed60182cf5/include/ggml/ggml.h#L237.
constexpr const uint64_t kGGMLComputationGraphNodesDefault = 2048;

// GGMLHashSize returns the size of the hash table for the given base,
// see https://github.com/ggerganov/ggml/blob/0cbb7c0e053f5419cfbebb46fbf4d4ed60182cf5/src/ggml.c#L17698-L17722.
uint64_t GGMLHashSize(uint64_t base) {
  // next primes after powers of two
  constexpr const size_t primes[] = {
      2,          3,         5,        11,        17,        37,
      67,         131,       257,      521,       1031,      2053,
      4099,       8209,      16411,    32771,     65537,     131101,
      262147,     524309,    1048583,  2097169,   4194319,   8388617,
      16777259,   33554467,  67108879, 134217757, 268435459, 536870923,
      1073741827, 2147483659};
  constexpr const size_t n_primes = sizeof(primes) / sizeof(primes[0]);

  // find the smallest prime that is larger or equal to base
  size_t l = 0;
  size_t r = n_primes;
  while (l < r) {
    size_t m = (l + r) / 2;
    if (primes[m] < base) {
      l = m + 1;
    } else {
      r = m;
    }
  }
  size_t sz = l < n_primes ? primes[l] : base | 1;
  return sz;
}

// GGMLComputationGraphOverhead is the overhead of GGML graph in bytes,
// see https://github.com/ggerganov/ggml/blob/0cbb7c0e053f5419cfbebb46fbf4d4ed60182cf5/src/ggml.c#L18905-L18917.
uint64_t GGMLComputationGraphOverhead(uint64_t nodes, bool grads) {
  const uint64_t pointer_size = 8;

  uint64_t g = kGGMLComputationGraphSize;
  g += pointer_size * nodes * 2;
  if (grads) {
    g += pointer_size * nodes;
  }
  g += pointer_size * GGMLHashSize(nodes);

  return kGGMLObjectSize + GGMLMemoryPadding(g);
}

}  // namespace hardware