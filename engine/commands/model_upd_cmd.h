#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "config/model_config.h"
namespace commands {
struct ModelUpdateOptions {
  std::string name;
  std::string model;
  std::string version;
  std::string stop;
  std::string top_p;
  std::string temperature;
  std::string frequency_penalty;
  std::string presence_penalty;
  std::string max_tokens;
  std::string stream;
  std::string ngl;
  std::string ctx_len;
  std::string engine;
  std::string prompt_template;
  std::string system_template;
  std::string user_template;
  std::string ai_template;
  std::string os;
  std::string gpu_arch;
  std::string quantization_method;
  std::string precision;
  std::string tp;
  std::string trtllm_version;
  std::string text_model;
  std::string files;
  std::string created;
  std::string object;
  std::string owned_by;
  std::string seed;
  std::string dynatemp_range;
  std::string dynatemp_exponent;
  std::string top_k;
  std::string min_p;
  std::string tfs_z;
  std::string typ_p;
  std::string repeat_last_n;
  std::string repeat_penalty;
  std::string mirostat;
  std::string mirostat_tau;
  std::string mirostat_eta;
  std::string penalize_nl;
  std::string ignore_eos;
  std::string n_probs;
  std::string min_keep;
  std::string grammar;
};
class ModelUpdCmd {
 public:
  ModelUpdCmd(std::string model_handle);
  void Exec(const ModelUpdateOptions& options);

 private:
  std::string model_handle_;
};
}  // namespace commands