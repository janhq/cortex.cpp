#pragma once

#include <sstream>
#include "common/tokenizer.h"

struct ModelMetadata {
  uint32_t version;
  uint64_t tensor_count;
  uint64_t metadata_kv_count;
  std::shared_ptr<Tokenizer> tokenizer;

  std::string ToString() const {
    std::ostringstream ss;
    ss << "ModelMetadata {\n"
       << "version: " << version << "\n"
       << "tensor_count: " << tensor_count << "\n"
       << "metadata_kv_count: " << metadata_kv_count << "\n"
       << "tokenizer: ";

    if (tokenizer) {
      ss << "\n" << tokenizer->ToString();
    } else {
      ss << "null";
    }

    ss << "\n}";
    return ss.str();
  }
};
