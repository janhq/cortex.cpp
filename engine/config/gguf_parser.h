#pragma once
#include <string>
#include "yaml_config.h"

namespace config {
constexpr char OPEN_CHAT_3_5_JINJA[] =
    R"({{ bos_token }}{% for message in messages %}{{ 'GPT4 Correct ' + "
    "message['role'].title() + ': ' + message['content'] + "
    "'<|end_of_turn|>'}}{% endfor %}{% if add_generation_prompt %}{{ 'GPT4 "
    "Correct Assistant:' }}{% endif %})";
constexpr char ZEPHYR_JINJA[] =
    R"({% for message in messages %}\n{% if message['role'] == 'user' %}\n{{ "
    "'<|user|>\n' + message['content'] + eos_token }}\n{% elif "
    "message['role'] == 'system' %}\n{{ '<|system|>\n' + message['content'] + "
    "eos_token }}\n{% elif message['role'] == 'assistant' %}\n{{ "
    "'<|assistant|>\n'  + message['content'] + eos_token }}\n{% endif "
    "%}\n{% if loop.last and add_generation_prompt %}\n{{ '<|assistant|>' "
    "}}\n{% endif %}\n{% endfor %})";
constexpr char LLAMA_3_1_JINJA[] =
    R"({% set loop_messages = messages %}{% for message in loop_messages "
    "%}{% set content = '<|start_header_id|>' + message['role'] + "
    "'<|end_header_id|>\n\n'+ message['content'] | trim + '<|eot_id|>' %}{% "
    "if loop.index0 == 0 %}{% set content = bos_token + content %}{% endif "
    "%}{{ content }}{% endfor %}{{ "
    "'<|start_header_id|>assistant<|end_header_id|>\n\n' }})";
constexpr char LLAMA_3_JINJA[] =
    R"({% set loop_messages = messages %}{% for message in loop_messages "
    "%}{% set content = '<|start_header_id|>' + message['role'] + "
    "'<|end_header_id|>\n\n'+ message['content'] | trim + '<|eot_id|>' %}{% "
    "if loop.index0 == 0 %}{% set content = bos_token + content %}{% endif "
    "%}{{ content }}{% endfor %}{% if add_generation_prompt %}{{ "
    "'<|start_header_id|>assistant<|end_header_id|>\n\n' }})";
constexpr uint32_t GGUF_MAGIC_NUMBER = 1179993927;

class GGUFHandler {
 public:
  void CloseFile();
  void Parse(const std::string& file_path);
  const ModelConfig& GetModelConfig() const;
  void PrintMetadata();

 private:
  std::pair<std::size_t, std::string> ReadString(std::size_t offset) const;
  size_t ReadMetadataValue(int type, std::size_t offset,
                           const std::string& key);
  size_t ReadArray(std::size_t offset, const std::string& key);
  void ModelConfigFromMetadata();
  void OpenFile(const std::string& file_path);
  void CheckOffset(size_t offset) const;

  uint8_t* data_;
  size_t file_size_;
  uint32_t version_;
  uint64_t tensor_count_;
  ModelConfig model_config_;
  std::unordered_map<std::string, uint8_t> metadata_uint8_;
  std::unordered_map<std::string, int8_t> metadata_int8_;
  std::unordered_map<std::string, uint16_t> metadata_uint16_;
  std::unordered_map<std::string, int16_t> metadata_int16_;
  std::unordered_map<std::string, uint32_t> metadata_uint32_;
  std::unordered_map<std::string, int32_t> metadata_int32_;
  std::unordered_map<std::string, float> metadata_float_;
  std::unordered_map<std::string, bool> metadata_bool_;
  std::unordered_map<std::string, std::string> metadata_string_;
  std::unordered_map<std::string, uint64_t> metadata_uint64_;
  std::unordered_map<std::string, int64_t> metadata_int64_;
  std::unordered_map<std::string, double> metadata_double_;
  std::unordered_map<std::string, std::vector<float>> metadata_array_float_;
  std::unordered_map<std::string, std::vector<std::string>>
      metadata_array_string_;
};
}  // namespace config
