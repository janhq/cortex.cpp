#pragma once

#include "common/json_serializable.h"

struct ModelTokenizer : JsonSerializable {
  std::string model;

  std::string pre;

  std::vector<std::string> tokens;

  std::vector<int> token_type;

  std::vector<std::string> merges;

  // TODO: clean this up
  size_t eos_token_id;

  size_t padding_token_id;

  size_t bos_token_id;

  bool add_bos_token;

  std::string chat_template;

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value root;
    // TODO: namh handle this

    return root;
  }
};
