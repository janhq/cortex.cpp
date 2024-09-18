#pragma once

#include <limits>
#include <string>
#include <vector>

namespace config {
struct ModelConfig {
  std::string name;
  std::string model;
  std::string version;
  std::vector<std::string> stop = {};
  float top_p = std::numeric_limits<float>::quiet_NaN();
  float temperature = std::numeric_limits<float>::quiet_NaN();
  float frequency_penalty = std::numeric_limits<float>::quiet_NaN();
  float presence_penalty = std::numeric_limits<float>::quiet_NaN();
  int max_tokens = std::numeric_limits<int>::quiet_NaN();
  bool stream = std::numeric_limits<bool>::quiet_NaN();
  int ngl = std::numeric_limits<int>::quiet_NaN();
  int ctx_len = std::numeric_limits<int>::quiet_NaN();
  std::string engine;
  std::string prompt_template;
  std::string system_template;
  std::string user_template;
  std::string ai_template;

  std::string os;
  std::string gpu_arch;
  std::string quantization_method;
  std::string precision;
  int tp = std::numeric_limits<int>::quiet_NaN();
  std::string trtllm_version;
  bool text_model = std::numeric_limits<bool>::quiet_NaN();
  std::string id;
  std::vector<std::string> files;
  std::size_t created;
  std::string object;
  std::string owned_by = "";

  int seed = std::numeric_limits<int>::quiet_NaN();
  float dynatemp_range = std::numeric_limits<float>::quiet_NaN();
  float dynatemp_exponent = std::numeric_limits<float>::quiet_NaN();
  int top_k = std::numeric_limits<int>::quiet_NaN();
  float min_p = std::numeric_limits<float>::quiet_NaN();
  float tfs_z = std::numeric_limits<float>::quiet_NaN();
  float typ_p = std::numeric_limits<float>::quiet_NaN();
  int repeat_last_n = std::numeric_limits<int>::quiet_NaN();
  float repeat_penalty = std::numeric_limits<float>::quiet_NaN();
  bool mirostat = std::numeric_limits<bool>::quiet_NaN();
  float mirostat_tau = std::numeric_limits<float>::quiet_NaN();
  float mirostat_eta = std::numeric_limits<float>::quiet_NaN();
  bool penalize_nl = std::numeric_limits<bool>::quiet_NaN();
  bool ignore_eos = std::numeric_limits<bool>::quiet_NaN();
  int n_probs = std::numeric_limits<int>::quiet_NaN();
  int min_keep = std::numeric_limits<int>::quiet_NaN();
  std::string grammar;
};
}  // namespace config
