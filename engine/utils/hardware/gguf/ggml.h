#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>

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
  bool is_quantized = false;
};

const std::unordered_map<GGMLType, GGMLTypeTrait> kGGMLTypeTraits = {
    {GGML_TYPE_F32, {1, 4, false}},       {GGML_TYPE_F16, {1, 2, false}},
    {GGML_TYPE_Q4_0, {32, 18, true}},     {GGML_TYPE_Q4_1, {32, 20, true}},
    {GGML_TYPE_Q5_0, {32, 22, true}},     {GGML_TYPE_Q5_1, {32, 24, true}},
    {GGML_TYPE_Q8_0, {32, 34, true}},     {GGML_TYPE_Q8_1, {32, 36, true}},
    {GGML_TYPE_Q2_K, {256, 84, true}},    {GGML_TYPE_Q3_K, {256, 110, true}},
    {GGML_TYPE_Q4_K, {256, 144, true}},   {GGML_TYPE_Q5_K, {256, 176, true}},
    {GGML_TYPE_Q6_K, {256, 210, true}},   {GGML_TYPE_Q8_K, {256, 292, true}},
    {GGML_TYPE_IQ2_XXS, {256, 66, true}}, {GGML_TYPE_IQ2_XS, {256, 74, true}},
    {GGML_TYPE_IQ3_XXS, {256, 98, true}}, {GGML_TYPE_IQ1_S, {256, 50, true}},
    {GGML_TYPE_IQ4_NL, {32, 18, true}},   {GGML_TYPE_IQ3_S, {256, 110, true}},
    {GGML_TYPE_IQ2_S, {256, 82, true}},   {GGML_TYPE_IQ4_XS, {256, 136, true}},
    {GGML_TYPE_I8, {1, 1, false}},        {GGML_TYPE_I16, {1, 2, false}},
    {GGML_TYPE_I32, {1, 4, false}},       {GGML_TYPE_I64, {1, 8, false}},
    {GGML_TYPE_F64, {1, 8, false}},       {GGML_TYPE_IQ1_M, {256, 56, true}},
    {GGML_TYPE_BF16, {1, 2, false}},      {GGML_TYPE_Q4_0_4_4, {32, 18, true}},
    {GGML_TYPE_Q4_0_4_8, {32, 18, true}}, {GGML_TYPE_Q4_0_8_8, {32, 18, true}},
    {GGML_TYPE_TQ1_0, {256, 54, true}},   {GGML_TYPE_TQ2_0, {256, 66, true}},
};

}  // namespace hardware
