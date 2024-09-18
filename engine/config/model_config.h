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

  int seed = -1;
  float dynatemp_range = 0.0f;
  float dynatemp_exponent = 1.0f;
  int top_k = 40;
  float min_p = 0.05f;
  float tfs_z = 1.0f;
  float typ_p = 1.0f;
  int repeat_last_n = 64;
  float repeat_penalty = 1.0f;
  bool mirostat = false;
  float mirostat_tau = 5.0f;
  float mirostat_eta = 0.1f;
  bool penalize_nl = false;
  bool ignore_eos = false;
  int n_probs = 0;
  int min_keep = 0;
  std::string grammar;
};
}  // namespace config
