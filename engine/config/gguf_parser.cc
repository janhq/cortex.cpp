#include <algorithm>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <limits>
#else
#include <sys/mman.h>  // For memory-mapped file
#include <unistd.h>    // For file descriptors
#endif

#include <fcntl.h>  // For file descriptors

#include "chat_template_renderer.h"

#include "gguf_parser.h"
#include "trantor/utils/Logger.h"
#include "utils/engine_constants.h"

namespace config {
#define NOMINMAX
constexpr int kDefaultMaxContextLength = 8192;

void GGUFHandler::OpenFile(const std::string& file_path) {
#ifdef _WIN32
  HANDLE file_handle_ = INVALID_HANDLE_VALUE;
  HANDLE file_mapping_ = nullptr;
  file_handle_ =
      CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file_handle_ == INVALID_HANDLE_VALUE) {
    throw std::runtime_error("Failed to open file");
  }
  // Get the file size
  LARGE_INTEGER file_size_struct;
  if (!GetFileSizeEx(file_handle_, &file_size_struct)) {
    CloseHandle(file_handle_);
    throw std::runtime_error("Failed to get file size");
  }
  file_size_ = static_cast<size_t>(file_size_struct.QuadPart);

  // Create a file mapping object
  file_mapping_ =
      CreateFileMappingA(file_handle_, nullptr, PAGE_READONLY, 0, 0, nullptr);
  if (file_mapping_ == nullptr) {
    CloseHandle(file_handle_);
    throw std::runtime_error("Failed to create file mapping");
  }

  // Map the file into memory
  data_ = static_cast<uint8_t*>(
      MapViewOfFile(file_mapping_, FILE_MAP_READ, 0, 0, file_size_));
  if (data_ == nullptr) {
    CloseHandle(file_mapping_);
    CloseHandle(file_handle_);
    throw std::runtime_error("Failed to map file");
  }

  // Close the file handle, as it is no longer needed after mapping
  CloseHandle(file_handle_);

#else
  file_size_ = std::filesystem::file_size(file_path);

  int file_descriptor = open(file_path.c_str(), O_RDONLY);
  // Memory-map the file
  data_ = static_cast<uint8_t*>(
      mmap(nullptr, file_size_, PROT_READ, MAP_PRIVATE, file_descriptor, 0));
  if (data_ == MAP_FAILED) {
    perror("Error mapping file");
    close(file_descriptor);
    throw std::runtime_error("Failed to map file");
  }

  close(file_descriptor);

#endif
}

void GGUFHandler::CloseFile() {
#ifdef _WIN32
  if (data_ != nullptr) {
    UnmapViewOfFile(data_);
    data_ = nullptr;
  }
#else
  if (data_ != nullptr && data_ != MAP_FAILED) {
    munmap(data_, file_size_);
  }
#endif
}

std::pair<std::size_t, std::string> GGUFHandler::ReadString(
    std::size_t offset) const {
  uint64_t length;
  std::memcpy(&length, data_ + offset, sizeof(uint64_t));

  if (offset + 8 + length > file_size_) {
    throw std::runtime_error(
        "GGUF metadata string length exceeds file size.\n");
  }

  std::string value(reinterpret_cast<const char*>(data_ + offset + 8), length);
  return {8 + static_cast<std::size_t>(length), value};
}

size_t GGUFHandler::ReadMetadataValue(int type, std::size_t offset,
                                      const std::string& key) {
  switch (type) {
    case 0:  // UINT8
      metadata_uint8_[key] = data_[offset];
      return 1;
    case 1:  // INT8
      metadata_int8_[key] = static_cast<int8_t>(data_[offset]);
      return 1;
    case 2:  // UINT16
      metadata_uint16_[key] =
          *reinterpret_cast<const uint16_t*>(data_ + offset);
      return 2;
    case 3:  // INT16
      metadata_int16_[key] = *reinterpret_cast<const int16_t*>(data_ + offset);
      return 2;
    case 4:  // UINT32
      metadata_uint32_[key] =
          *reinterpret_cast<const uint32_t*>(data_ + offset);
      return 4;
    case 5:  // INT32
      metadata_int32_[key] = *reinterpret_cast<const int32_t*>(data_ + offset);
      return 4;
    case 6:  // FLOAT32
      metadata_float_[key] = *reinterpret_cast<const float*>(data_ + offset);
      return 4;
    case 7:  // BOOL
      metadata_bool_[key] = data_[offset] != 0;
      return 1;
    case 8:  // STRING
    {
      auto [byte_length, value] = ReadString(offset);
      metadata_string_[key] = value;
      return byte_length;
    }
    case 9:  // ARRAY

      return ReadArray(offset, key);
    case 10:  // UINT64
      metadata_uint64_[key] =
          *reinterpret_cast<const uint64_t*>(data_ + offset);
      return 8;
    case 11:  // INT64
      metadata_int64_[key] = *reinterpret_cast<const int64_t*>(data_ + offset);
      return 8;
    case 12:  // FLOAT64
      metadata_double_[key] = *reinterpret_cast<const double*>(data_ + offset);
      return 8;
    default:
      throw std::runtime_error("Unsupported metadata type: " +
                               std::to_string(type));
  }
}

size_t GGUFHandler::ReadArray(std::size_t offset, const std::string& key) {
  uint32_t array_type = *reinterpret_cast<const uint32_t*>(data_ + offset);
  // std::memcpy(&array_type, data_ + offset, sizeof(uint32_t));

  uint64_t array_length =
      *reinterpret_cast<const uint64_t*>(data_ + offset + 4);
  // std::memcpy(&array_length, data_ + offset + 4, sizeof(uint64_t));
  LOG_INFO << "\n"
           << "Parsing array type: " << array_type
           << ", array length:" << array_length << "\n";
  std::size_t array_offset = 12;
  std::vector<std::string> array_values_string;
  std::vector<float> array_values_float;
  uint8_t uint8_value;
  int8_t int8_value;
  uint16_t uint16_value;
  int16_t int16_value;
  uint32_t uint32_value;
  int32_t int32_value;
  float float_value;
  bool bool_value;
  std::string string_value;
  uint64_t uint64_value;
  int64_t int64_value;
  double double_value;
  size_t length;

  for (uint64_t i = 0; i < array_length; ++i) {
    // auto [byteLength, value] = ReadMetadataValue(array_type, offset + array_offset);
    // assume that array ony has 2 type string and int
    switch (array_type) {
      case 0:
        uint8_value = data_[offset + array_offset];
        length = 1;
        array_values_float.push_back(static_cast<float>(uint8_value));
        break;
      case 1: {
        int8_value = static_cast<int8_t>(data_[offset + array_offset]);
        length = 1;
        array_values_float.push_back(static_cast<float>(int8_value));
      }

      break;
      case 2:
        uint16_value =
            *reinterpret_cast<const uint16_t*>(data_ + offset + array_offset);
        length = 2;
        array_values_float.push_back(static_cast<float>(uint16_value));
        break;
      case 3:
        int16_value =
            *reinterpret_cast<const int16_t*>(data_ + offset + array_offset);
        length = 2;
        array_values_float.push_back(static_cast<float>(int16_value));
        break;
      case 4:
        uint32_value =
            *reinterpret_cast<const uint32_t*>(data_ + offset + array_offset);
        length = 4;
        array_values_float.push_back(static_cast<float>(uint32_value));
        break;
      case 5:
        int32_value =
            *reinterpret_cast<const int32_t*>(data_ + offset + array_offset);
        length = 4;
        array_values_float.push_back(static_cast<float>(int32_value));
        break;
      case 6:
        float_value =
            *reinterpret_cast<const float*>(data_ + offset + array_offset);
        length = 4;
        array_values_float.push_back(static_cast<float>(float_value));
        break;
      case 7:
        bool_value = data_[offset + array_offset] != 0;
        length = 1;
        array_values_float.push_back(static_cast<float>(bool_value));
        break;
      case 8: {
        uint64_t length_ =
            *reinterpret_cast<const uint64_t*>(data_ + offset + array_offset);
        std::string value(
            reinterpret_cast<const char*>(data_ + offset + array_offset + 8),
            length_);
        length = 8 + static_cast<std::size_t>(length_);
        array_values_string.push_back(value);
      } break;
      case 10:
        uint64_value =
            *reinterpret_cast<const uint64_t*>(data_ + offset + array_offset);
        length = 8;
        array_values_float.push_back(static_cast<float>(uint64_value));
        break;
      case 11:
        int64_value =
            *reinterpret_cast<const int64_t*>(data_ + offset + array_offset);
        length = 8;
        array_values_float.push_back(static_cast<float>(int64_value));
        break;
      case 12:
        double_value =
            *reinterpret_cast<const double*>(data_ + offset + array_offset);
        length = 8;
        array_values_float.push_back(static_cast<float>(double_value));
        break;
      default:
        throw std::runtime_error("Unsupported metadata type: " +
                                 std::to_string(array_type));
        break;
    }

    array_offset += length;
    if (offset + array_offset > file_size_) {
      throw std::runtime_error("GGUF Parser Array exceeded file size.\n");
    }
  }
  if (array_values_string.size() > 0)
    metadata_array_string_[key] = array_values_string;
  else
    metadata_array_float_[key] = array_values_float;
  return array_offset;
}

void GGUFHandler::Parse(const std::string& file_path) {
  OpenFile(file_path);
  LOG_INFO << "GGUF magic number: " << *reinterpret_cast<const uint32_t*>(data_)
           << "\n";
  if (*reinterpret_cast<const uint32_t*>(data_) != GGUF_MAGIC_NUMBER) {
    throw std::runtime_error("Not a valid GGUF file");
  }

  version_ = *reinterpret_cast<const uint32_t*>(data_ + 4);
  tensor_count_ = *reinterpret_cast<const uint64_t*>(data_ + 8);
  uint64_t metadata_kv_count = *reinterpret_cast<const uint64_t*>(data_ + 16);
  LOG_INFO << "version: " << version_ << "\ntensor count: " << tensor_count_
           << "\nmetadata key-value pairs: " << metadata_kv_count << "\n";

  std::size_t offset = 24;

  for (uint64_t i = 0; i < metadata_kv_count; ++i) {
    LOG_INFO << "Parsing key-value number " << i << "\n";
    auto [key_byte_length, key] = ReadString(offset);
    offset += key_byte_length;
    LOG_INFO << "key: " << key << "\n";
    uint32_t value_type = *reinterpret_cast<const uint32_t*>(data_ + offset);
    offset += 4;
    LOG_INFO << "value type number: " << value_type << "\n";
    size_t value_byte_length = ReadMetadataValue(value_type, offset, key);
    offset += value_byte_length;
    LOG_INFO << "-------------------------------------------- " << "\n";
  }
  try {
    PrintMetadata();
  } catch (const std::exception& e) {
    LOG_ERROR << "Error parsing metadata: " << e.what() << "\n";
  }
  ModelConfigFromMetadata();
  CloseFile();
}

void GGUFHandler::PrintMetadata() {
  LOG_INFO << "GGUF Metadata:" << "\n";
  for (const auto& [key, value] : metadata_uint8_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_int8_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_uint16_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_int16_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_uint32_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_int32_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_float_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_bool_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_string_) {

    if (key.compare("tokenizer.chat_template") == 0) {
      LOG_INFO << key << ": " << "\n" << value << "\n";

      std::vector<llama_chat_msg> messages{
          llama_chat_msg{"system", "{system_message}"},
          llama_chat_msg{"user", "{prompt}"}};
      std::string result = llama_chat_apply_template(value, messages, true);
      LOG_INFO << "result jinja render: " << result << "\n";
    } else {
      LOG_INFO << key << ": " << value << "\n";
    }
  }

  for (const auto& [key, value] : metadata_uint64_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_int64_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_double_)
    LOG_INFO << key << ": " << value << "\n";

  for (const auto& [key, value] : metadata_array_float_)
    LOG_INFO << key << "num elements: " << value.size() << "\n";

  for (const auto& [key, value] : metadata_array_string_)
    LOG_INFO << key << " num elements: " << value.size() << "\n";
}

void GGUFHandler::ModelConfigFromMetadata() {
  int eos_token, bos_token, max_tokens, version, ngl;
  std::string chat_template, name, eos_string, bos_string;
  std::vector<std::string> tokens, stop;
  model_config_.top_p = 0.95;
  model_config_.temperature = 0.7;
  model_config_.frequency_penalty = 0;
  model_config_.presence_penalty = 0;
  model_config_.stream = true;
  model_config_.engine = kLlamaEngine;
  model_config_.created = std::time(nullptr);
  model_config_.model = "model";
  model_config_.owned_by = "";
  model_config_.seed = -1;
  model_config_.dynatemp_range = 0.0f;
  model_config_.dynatemp_exponent = 1.0f;
  model_config_.top_k = 40;
  model_config_.min_p = 0.05f;
  model_config_.tfs_z = 1.0f;
  model_config_.typ_p = 1.0f;
  model_config_.repeat_last_n = 64;
  model_config_.repeat_penalty = 1.0f;
  model_config_.mirostat = false;
  model_config_.mirostat_tau = 5.0f;
  model_config_.mirostat_eta = 0.1f;
  model_config_.penalize_nl = false;
  model_config_.ignore_eos = false;
  model_config_.n_probs = 0;
  model_config_.min_keep = 0;
  model_config_.grammar = "";

  // Get version, bos, eos id, contex_len, ngl from meta data
  for (const auto& [key, value] : metadata_uint8_) {
    if (key.compare("general.quantization_version") == 0)
      version = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.bos_token_id") == 0)
      bos_token = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.eos_token_id") == 0)
      eos_token = static_cast<int>(value);
    else if (key.find("context_length") != std::string::npos)
      max_tokens = static_cast<int>(value);
    else if (key.find("block_count") != std::string::npos)
      ngl = static_cast<int>(value) + 1;
  }

  for (const auto& [key, value] : metadata_int8_) {
    if (key.compare("general.quantization_version") == 0)
      version = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.bos_token_id") == 0)
      bos_token = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.eos_token_id") == 0)
      eos_token = static_cast<int>(value);
    else if (key.find("context_length") != std::string::npos)
      max_tokens = static_cast<int>(value);
    else if (key.find("block_count") != std::string::npos)
      ngl = static_cast<int>(value) + 1;
  }

  for (const auto& [key, value] : metadata_uint16_) {
    if (key.compare("general.quantization_version") == 0)
      version = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.bos_token_id") == 0)
      bos_token = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.eos_token_id") == 0)
      eos_token = static_cast<int>(value);
    else if (key.find("context_length") != std::string::npos)
      max_tokens = static_cast<int>(value);
    else if (key.find("block_count") != std::string::npos)
      ngl = static_cast<int>(value) + 1;
  }

  for (const auto& [key, value] : metadata_int16_) {
    if (key.compare("general.quantization_version") == 0)
      version = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.bos_token_id") == 0)
      bos_token = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.eos_token_id") == 0)
      eos_token = static_cast<int>(value);
    else if (key.find("context_length") != std::string::npos)
      max_tokens = static_cast<int>(value);
    else if (key.find("block_count") != std::string::npos)
      ngl = static_cast<int>(value) + 1;
  }

  for (const auto& [key, value] : metadata_uint32_) {
    if (key.compare("general.quantization_version") == 0)
      version = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.bos_token_id") == 0)
      bos_token = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.eos_token_id") == 0)
      eos_token = static_cast<int>(value);
    else if (key.find("context_length") != std::string::npos)
      max_tokens = static_cast<int>(value);
    else if (key.find("block_count") != std::string::npos)
      ngl = static_cast<int>(value) + 1;
  }

  for (const auto& [key, value] : metadata_int32_) {
    if (key.compare("general.quantization_version") == 0)
      version = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.bos_token_id") == 0)
      bos_token = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.eos_token_id") == 0)
      eos_token = static_cast<int>(value);
    else if (key.find("context_length") != std::string::npos)
      max_tokens = static_cast<int>(value);
    else if (key.find("block_count") != std::string::npos)
      ngl = static_cast<int>(value) + 1;
  }
  for (const auto& [key, value] : metadata_uint64_) {
    if (key.compare("general.quantization_version") == 0)
      version = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.bos_token_id") == 0)
      bos_token = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.eos_token_id") == 0)
      eos_token = static_cast<int>(value);
    else if (key.find("context_length") != std::string::npos)
      max_tokens = static_cast<int>(value);
    else if (key.find("block_count") != std::string::npos)
      ngl = static_cast<int>(value) + 1;
  }

  for (const auto& [key, value] : metadata_int64_) {
    if (key.compare("general.quantization_version") == 0)
      version = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.bos_token_id") == 0)
      bos_token = static_cast<int>(value);
    else if (key.compare("tokenizer.ggml.eos_token_id") == 0)
      eos_token = static_cast<int>(value);
    else if (key.find("context_length") != std::string::npos)
      max_tokens = static_cast<int>(value);
    else if (key.find("block_count") != std::string::npos)
      ngl = static_cast<int>(value) + 1;
  }
  for (const auto& [key, value] : metadata_array_string_) {
    if (key.compare("tokenizer.ggml.tokens") == 0) {
      tokens = std::move(value);
    }
  }
  for (const auto& [key, value] : metadata_string_) {
    if (key.compare("general.name") == 0) {
      name = std::regex_replace(value, std::regex(" "), "-");
    } else if (key.find("chat_template") != std::string::npos) {
      if (value.compare(ZEPHYR_JINJA) == 0) {
        chat_template =
            "<|system|>\n{system_message}</s>\n<|user|>\n{prompt}</"
            "s>\n<|assistant|>\n";
      } else if (value.compare(OPEN_CHAT_3_5_JINJA) == 0) {
        chat_template =
            "GPT4 Correct User: {prompt}<|end_of_turn|>GPT4 Correct Assistant:";
      } else if (value.compare(LLAMA_3_JINJA) == 0 ||
                 value.compare(LLAMA_3_1_JINJA) == 0) {
        chat_template =
            "<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n\n{"
            "system_message}<|eot_id|><|start_header_id|>user<|end_header_id|>"
            "\n\n{prompt}<|eot_id|><|start_header_id|>assistant<|end_header_id|"
            ">\n\n";
      } else {
        try {
          std::vector<llama_chat_msg> messages{
              llama_chat_msg{"system", "{system_message}"},
              llama_chat_msg{"user", "{prompt}"}};
          chat_template = llama_chat_apply_template(value, messages, true);
        } catch (const std::exception& e) {
          std::cerr << "Error render chat template: " << e.what()
                    << ". Using default template: \n[INST] "
                       "<<SYS>>\n{system_message}\n<</SYS>>\n{prompt}[/INST]"
                    << "\n";
          chat_template =
              "[INST] <<SYS>>\n{system_message}\n<</SYS>>\n{prompt}[/INST]";
        }
      }
    }
  }

  try {
    if (tokens.size() > (unsigned)eos_token) {
      eos_string = tokens[eos_token];
      stop.push_back(std::move(eos_string));
    } else {
      LOG_ERROR << "Can't find stop token";
    }
  } catch (const std::exception& e) {
    LOG_ERROR << "Can't find stop token";
  }

  model_config_.stop = std::move(stop);
  if (chat_template.empty())
    chat_template =
        "[INST] <<SYS>>\n{system_message}\n<</SYS>>\n{prompt}[/INST]";
  model_config_.prompt_template = std::move(chat_template);
  model_config_.name = name;
  model_config_.model = name;
  model_config_.id = name;
  model_config_.version = std::to_string(version);
  model_config_.max_tokens = max_tokens;
  model_config_.ctx_len = max_tokens;
  model_config_.ngl = ngl;
  (void)bos_token;
}

const ModelConfig& GGUFHandler::GetModelConfig() const {
  return model_config_;
}
}  // namespace config
