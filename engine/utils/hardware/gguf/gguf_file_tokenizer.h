#pragma once

#include <cstdint>
#include <string>

namespace hardware {
struct GGUFTokenizer {
  std::string model;             // Model of the tokenizer
  uint64_t tokens_length;        // Size of tokens
  uint64_t merges_length;        // Size of merges
  uint64_t added_tokens_length;  // Size of added tokens after training
  int64_t bos_token_id;          // ID of the beginning of sentence token
  int64_t eos_token_id;          // ID of the end of sentence token
  int64_t eot_token_id;          // ID of the end of text token
  int64_t eom_token_id;          // ID of the end of message token
  int64_t unknown_token_id;      // ID of the unknown token
  int64_t separator_token_id;    // ID of the separator token
  int64_t padding_token_id;      // ID of the padding token

  // Appendix
  int64_t token_size;   // Size of tokens in bytes
  int64_t merges_size;  // Size of merges in bytes
};
}  // namespace hardware