#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>
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

inline float GetQuantBit(GGMLType gt) {
  switch (gt) {
    case GGML_TYPE_I32:
    case GGML_TYPE_F32:
      return 32.0f;
    case GGML_TYPE_I16:
    case GGML_TYPE_BF16:
    case GGML_TYPE_F16:
      return 16.0f;
    case GGML_TYPE_IQ2_S:
    case GGML_TYPE_IQ2_XXS:
    case GGML_TYPE_IQ2_XS:
      return 2.31f;
    case GGML_TYPE_Q2_K:
      return 2.5625f;
    case GGML_TYPE_IQ3_XXS:
    case GGML_TYPE_IQ3_S:
    case GGML_TYPE_Q3_K:
      return 3.4375f;
    case GGML_TYPE_Q4_0_4_4:
    case GGML_TYPE_Q4_0_4_8:
    case GGML_TYPE_Q4_0_8_8:
    case GGML_TYPE_IQ4_NL:
    case GGML_TYPE_IQ4_XS:
    case GGML_TYPE_Q4_0:
    case GGML_TYPE_Q4_1:
    case GGML_TYPE_Q4_K:
      return 4.5f;
    case GGML_TYPE_Q5_0:
    case GGML_TYPE_Q5_1:
    case GGML_TYPE_Q5_K:
      return 5.5f;
    case GGML_TYPE_Q6_K:
      return 6.5625f;
    case GGML_TYPE_I8:
    case GGML_TYPE_Q8_0:
    case GGML_TYPE_Q8_1:
    case GGML_TYPE_Q8_K:
      return 8.0f;

    case GGML_TYPE_I64:
    case GGML_TYPE_F64:
      return 64.0f;

    default:
      return 8.0f;
  }
}

inline std::string to_string(GGMLType t) {
  switch (t) {
    case GGML_TYPE_F32:
      return "F32";
    case GGML_TYPE_F16:
      return "F16";
    case GGML_TYPE_Q4_0:
      return "Q4_0";
    case GGML_TYPE_Q4_1:
      return "Q4_1";
    case GGML_TYPE_Q5_0:
      return "Q5_0";
    case GGML_TYPE_Q5_1:
      return "Q5_1";
    case GGML_TYPE_Q8_0:
      return "Q8_0";
    case GGML_TYPE_Q8_1:
      return "Q8_1";
    case GGML_TYPE_Q2_K:
      return "Q2_K";
    case GGML_TYPE_Q3_K:
      return "Q3_K";
    case GGML_TYPE_Q4_K:
      return "Q4_K";
    case GGML_TYPE_Q5_K:
      return "Q5_K";
    case GGML_TYPE_Q6_K:
      return "Q6_K";
    case GGML_TYPE_Q8_K:
      return "Q8_K";
    case GGML_TYPE_IQ2_XXS:
      return "IQ2_XXS";
    case GGML_TYPE_IQ2_XS:
      return "IQ2_XS";
    case GGML_TYPE_IQ3_XXS:
      return "IQ3_XXS";
    case GGML_TYPE_IQ1_S:
      return "IQ1_S";
    case GGML_TYPE_IQ4_NL:
      return "IQ4_NL";
    case GGML_TYPE_IQ3_S:
      return "IQ3_S";
    case GGML_TYPE_IQ2_S:
      return "IQ2_S";
    case GGML_TYPE_IQ4_XS:
      return "IQ4_XS";
    case GGML_TYPE_I8:
      return "I8";
    case GGML_TYPE_I16:
      return "I16";
    case GGML_TYPE_I32:
      return "I32";
    case GGML_TYPE_I64:
      return "I64";
    case GGML_TYPE_F64:
      return "F64";
    case GGML_TYPE_IQ1_M:
      return "IQ1_M";
    case GGML_TYPE_BF16:
      return "BF16";
    case GGML_TYPE_Q4_0_4_4:
      return "Q4_0_4_4";
    case GGML_TYPE_Q4_0_4_8:
      return "Q4_0_4_8";
    case GGML_TYPE_Q4_0_8_8:
      return "Q4_0_8_8";
    case GGML_TYPE_TQ1_0:
      return "TQ1_0";
    case GGML_TYPE_TQ2_0:
      return "TQ2_0";
    default:
      return "Invalid";
  }
}

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
}  // namespace hardware
