#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <variant>
#include "common/model_metadata.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"

/**
 * Parsing the GGUF metadata.
 *
 * Reference: https://github.com/ggerganov/ggml/blob/master/docs/gguf.md
 */
namespace cortex_utils {
namespace {
// present in the first 4 bytes of a GGUF file
constexpr uint32_t GGUF_MAGIC_NUMBER = 1179993927;

constexpr static auto GGUF_VERSION_LENGTH = 4;
constexpr static auto TENSOR_COUNT_LENGTH = 8;
constexpr static auto METADATA_KV_COUNT = 8;

constexpr static auto TOKEN_LIST_KEY = "tokenizer.ggml.tokens";
constexpr static auto BOS_ID_KEY = "tokenizer.ggml.bos_token_id";
constexpr static auto EOS_ID_KEY = "tokenizer.ggml.eos_token_id";
constexpr static auto UNK_ID_KEY = "tokenizer.ggml.unknown_token_id";
constexpr static auto PADDING_ID_KEY = "tokenizer.ggml.padding_token_id";

constexpr static auto CHAT_TEMPLATE_ID_KEY = "tokenizer.chat_template";
constexpr static auto ADD_BOS_TOKEN_KEY = "tokenizer.ggml.add_bos_token";
constexpr static auto ADD_EOS_TOKEN_KEY = "tokenizer.ggml.add_eos_token";
const std::vector<std::string> kSpecialTokenIds{BOS_ID_KEY, EOS_ID_KEY,
                                                UNK_ID_KEY, PADDING_ID_KEY};

struct MetadataArrayElement;

// clang-format off
using MetadataValue = std::variant<
    uint8_t, int8_t,
    uint16_t, int16_t,
    uint32_t, int32_t,
    uint64_t, int64_t,
    float, double,
    bool, std::string,
    std::vector<MetadataArrayElement>
>;

// clang-format on

struct MetadataArrayElement {
  MetadataValue value;

  // Add constructors for different types
  MetadataArrayElement(uint8_t v) : value(v) {}
  MetadataArrayElement(int8_t v) : value(v) {}
  MetadataArrayElement(uint16_t v) : value(v) {}
  MetadataArrayElement(int16_t v) : value(v) {}
  MetadataArrayElement(uint32_t v) : value(v) {}
  MetadataArrayElement(int32_t v) : value(v) {}
  MetadataArrayElement(uint64_t v) : value(v) {}
  MetadataArrayElement(int64_t v) : value(v) {}
  MetadataArrayElement(float v) : value(v) {}
  MetadataArrayElement(double v) : value(v) {}
  MetadataArrayElement(bool v) : value(v) {}
  MetadataArrayElement(const std::string& v) : value(v) {}
  MetadataArrayElement(std::string&& v) : value(std::move(v)) {}

  MetadataArrayElement(MetadataValue&& v) : value(std::move(v)) {}
};

struct MetadataValueResult {
  size_t bytes_read;
  MetadataValue value;

  template <typename T>
  MetadataValueResult(size_t br, T&& val)
      : bytes_read(br), value(std::forward<T>(val)) {}
};

std::pair<std::size_t, std::string> ReadString(std::ifstream& file) {
  uint64_t length;
  file.read(reinterpret_cast<char*>(&length), sizeof(uint64_t));

  if (!file) {
    throw std::runtime_error("Failed to read string length");
  }

  if (length > 1024 * 1024 * 1024) {
    throw std::runtime_error("String length too large: " +
                             std::to_string(length));
  }

  std::string value(length, '\0');
  file.read(value.data(), length);

  if (!file) {
    throw std::runtime_error("Failed to read string content of length " +
                             std::to_string(length));
  }

  return {8 + length, value};
}

inline MetadataValueResult ReadMetadataValue(uint32_t value_type,
                                             std::ifstream& file,
                                             const std::string& key) {
  switch (value_type) {
    case 0: {  // uint8
      uint8_t value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(uint8_t), value};
    }
    case 1: {  // int8
      int8_t value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(int8_t), value};
    }
    case 2: {  // uint16
      uint16_t value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(uint16_t), value};
    }
    case 3: {  // int16
      int16_t value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(int16_t), value};
    }
    case 4: {  // uint32
      uint32_t value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(uint32_t), value};
    }
    case 5: {  // int32
      int32_t value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(int32_t), value};
    }
    case 6: {  // float32
      float value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(float), value};
    }
    case 7: {  // bool
      bool value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(bool), value};
    }
    case 8: {  // string
      auto [length, value] = ReadString(file);
      return {length, value};
    }
    case 9: {  // array
      uint32_t array_type;
      file.read(reinterpret_cast<char*>(&array_type), sizeof(uint32_t));

      uint64_t array_length;
      file.read(reinterpret_cast<char*>(&array_length), sizeof(uint64_t));

      size_t bytes_read = 12;  // 4 for type + 8 for length

      std::vector<std::string> array_values_string;
      std::vector<float> array_values_float;

      for (uint64_t i = 0; i < array_length; ++i) {
        auto result = ReadMetadataValue(array_type, file,
                                        key + "[" + std::to_string(i) + "]");
        bytes_read += result.bytes_read;

        if (array_type == 8) {
          array_values_string.push_back(std::get<std::string>(result.value));
        } else {
          float float_value;
          switch (result.value.index()) {
            case 0:
              float_value = static_cast<float>(std::get<uint8_t>(result.value));
              break;
            case 1:
              float_value = static_cast<float>(std::get<int8_t>(result.value));
              break;
            case 2:
              float_value =
                  static_cast<float>(std::get<uint16_t>(result.value));
              break;
            case 3:
              float_value = static_cast<float>(std::get<int16_t>(result.value));
              break;
            case 4:
              float_value =
                  static_cast<float>(std::get<uint32_t>(result.value));
              break;
            case 5:
              float_value = static_cast<float>(std::get<int32_t>(result.value));
              break;
            case 6:
              float_value =
                  static_cast<float>(std::get<uint64_t>(result.value));
              break;
            case 7:
              float_value = static_cast<float>(std::get<int64_t>(result.value));
              break;
            case 8:
              float_value = std::get<float>(result.value);
              break;
            case 9:
              float_value = static_cast<float>(std::get<double>(result.value));
              break;
            case 10:
              float_value = static_cast<float>(std::get<bool>(result.value));
              break;
            default:
              throw std::runtime_error(
                  "Unexpected type in array element conversion");
          }
          array_values_float.push_back(float_value);
        }
      }

      if (!array_values_string.empty()) {
        std::vector<MetadataArrayElement> result;
        result.reserve(array_values_string.size());
        for (const auto& str : array_values_string) {
          result.emplace_back(str);
        }
        return {bytes_read, std::move(result)};
      } else {
        std::vector<MetadataArrayElement> result;
        result.reserve(array_values_float.size());
        for (float val : array_values_float) {
          result.emplace_back(val);
        }
        return {bytes_read, std::move(result)};
      }
    }

    case 10: {  // uint64
      uint64_t value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(uint64_t), value};
    }
    case 11: {  // int64
      int64_t value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(int64_t), value};
    }
    case 12: {  // float64/double
      double value;
      file.read(reinterpret_cast<char*>(&value), sizeof(value));
      return {sizeof(double), value};
    }
    default:
      throw std::runtime_error("Unknown value type: " +
                               std::to_string(value_type) + " for key: " + key);
  }
}

void PrintMetadataValue(const std::string& key, const MetadataValue& value) {
  std::ostringstream oss;
  oss << "Key: " << key << " = ";

  switch (value.index()) {
    case 0:  // uint8_t
      oss << "uint8: " << static_cast<int>(std::get<uint8_t>(value));
      break;
    case 1:  // int8_t
      oss << "int8: " << static_cast<int>(std::get<int8_t>(value));
      break;
    case 2:  // uint16_t
      oss << "uint16: " << std::get<uint16_t>(value);
      break;
    case 3:  // int16_t
      oss << "int16: " << std::get<int16_t>(value);
      break;
    case 4:  // uint32_t
      oss << "uint32: " << std::get<uint32_t>(value);
      break;
    case 5:  // int32_t
      oss << "int32: " << std::get<int32_t>(value);
      break;
    case 6:  // uint64_t
      oss << "uint64: " << std::get<uint64_t>(value);
      break;
    case 7:  // int64_t
      oss << "int64: " << std::get<int64_t>(value);
      break;
    case 8:  // float
      oss << "float: " << std::get<float>(value);
      break;
    case 9:  // double
      oss << "double: " << std::get<double>(value);
      break;
    case 10:  // bool
      oss << "bool: " << (std::get<bool>(value) ? "true" : "false");
      break;
    case 11:  // string
      oss << "string: " << std::get<std::string>(value);
      break;
    case 12: {  // vector<MetadataArrayElement>
      const auto& arr = std::get<std::vector<MetadataArrayElement>>(value);
      oss << "array[" << arr.size() << "]: {";
      for (size_t i = 0; i < arr.size(); ++i) {
        if (i > 0)
          oss << ", ";
        std::ostringstream key_oss;
        key_oss << key << "[" << i << "]";
        PrintMetadataValue(key_oss.str(), arr[i].value);
      }
      oss << "}";
      break;
    }
  }

  CTL_INF(oss.str());
}
}  // namespace

inline cpp::result<std::shared_ptr<ModelMetadata>, std::string>
ReadGgufMetadata(const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) {
    return cpp::fail("Gguf file does not exist at " + path.string());
  }

  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return cpp::fail("Failed to open file: " + path.string());
  }

  uint32_t magic_number;
  file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
  if (magic_number != GGUF_MAGIC_NUMBER) {
    return cpp::fail("Invalid GGUF file: incorrect magic number");
  }

  auto metadata_ptr = std::make_shared<ModelMetadata>();

  uint32_t version;
  file.read(reinterpret_cast<char*>(&version), GGUF_VERSION_LENGTH);
  metadata_ptr->version = version;

  uint64_t tensor_count;
  file.read(reinterpret_cast<char*>(&tensor_count), TENSOR_COUNT_LENGTH);
  metadata_ptr->tensor_count = tensor_count;

  uint64_t metadata_kv_count;
  file.read(reinterpret_cast<char*>(&metadata_kv_count), METADATA_KV_COUNT);
  metadata_ptr->metadata_kv_count = metadata_kv_count;

  std::unordered_map<std::string, MetadataValueResult> kv;
  for (uint64_t i = 0; i < metadata_kv_count; ++i) {
    auto [key_byte_length, key] = ReadString(file);

    char value_type_bytes[4];
    file.read(value_type_bytes, 4);
    uint32_t value_type =
        static_cast<uint32_t>(static_cast<unsigned char>(value_type_bytes[0])) |
        (static_cast<uint32_t>(static_cast<unsigned char>(value_type_bytes[1]))
         << 8) |
        (static_cast<uint32_t>(static_cast<unsigned char>(value_type_bytes[2]))
         << 16) |
        (static_cast<uint32_t>(static_cast<unsigned char>(value_type_bytes[3]))
         << 24);

    try {
      auto result = ReadMetadataValue(value_type, file, key);
      kv.emplace(key, result);
    } catch (const std::exception& e) {
      CTL_ERR("Error reading metadata value for key '" + key +
              "': " + e.what());
      return cpp::fail("Error reading metadata value for key '" + key + "'");
    }
  }

  {
    metadata_ptr->tokenizer = std::make_shared<GgufTokenizer>();
    // initialize tokenizer
    if (auto it = kv.find(CHAT_TEMPLATE_ID_KEY); it != kv.end()) {
      metadata_ptr->tokenizer->chat_template =
          std::get<std::string>(it->second.value);
    }

    for (const auto& key : kSpecialTokenIds) {
      if (auto it = kv.find(key); it != kv.end()) {
        auto id = std::get<uint32_t>(it->second.value);
        if (auto token_it = kv.find(TOKEN_LIST_KEY); token_it != kv.end()) {
          auto& tokens = std::get<std::vector<MetadataArrayElement>>(
              token_it->second.value);

          if (key == BOS_ID_KEY) {
            metadata_ptr->tokenizer->bos_token =
                std::get<std::string>(tokens[id].value);
          } else if (key == EOS_ID_KEY) {
            metadata_ptr->tokenizer->eos_token =
                std::get<std::string>(tokens[id].value);
          } else if (key == UNK_ID_KEY) {
            metadata_ptr->tokenizer->unknown_token =
                std::get<std::string>(tokens[id].value);
          } else if (key == PADDING_ID_KEY) {
            metadata_ptr->tokenizer->padding_token =
                std::get<std::string>(tokens[id].value);
          } else {
            CTL_ERR("Unknown special token key: " + key);
          }
        }
      }
    }

    if (auto it = kv.find(ADD_BOS_TOKEN_KEY); it != kv.end()) {
      metadata_ptr->tokenizer->add_bos_token = std::get<bool>(it->second.value);
    }

    if (auto it = kv.find(ADD_EOS_TOKEN_KEY); it != kv.end()) {
      metadata_ptr->tokenizer->add_eos_token = std::get<bool>(it->second.value);
    }
  }

  CTL_INF("Parsed GGUF metadata successfully: " + metadata_ptr->ToString());
  return metadata_ptr;
}
}  // namespace cortex_utils
