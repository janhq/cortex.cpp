#pragma once
#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <any>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <limits>
#else
#include <sys/mman.h>  // For memory-mapped file
#include <unistd.h>    // For file descriptors
#endif

#include "ggml.h"
#include "gguf_file_architecture.h"
#include "gguf_file_tokenizer.h"
#include "gguf_scalar.h"
#include "utils/string_utils.h"

namespace hardware {
#undef min
#undef max

using GGUFMagic = uint32_t;
constexpr const GGUFMagic kGGUFMagicGGML = 0x67676d6c;
constexpr const GGUFMagic kGGUFMagicGGMF = 0x67676d66;
constexpr const GGUFMagic kGGUFMagicGGJT = 0x67676a74;
constexpr const GGUFMagic kGGUFMagicGGUFLe = 0x46554747;  // GGUF
constexpr const GGUFMagic kGGUFMagicGGUFBe = 0x47475546;  // GGUF

using GGUFVersion = uint32_t;
constexpr const GGUFVersion kGGUFVersionV1 = 1;
constexpr const GGUFVersion kGGUFVersionV2 = 2;
constexpr const GGUFVersion kGGUFVersionV3 = 3;

enum GGUFMetadataValueType : uint32_t {
  GGUFMetadataValueTypeUint8 = 0,
  GGUFMetadataValueTypeInt8,
  GGUFMetadataValueTypeUint16,
  GGUFMetadataValueTypeInt16,
  GGUFMetadataValueTypeUint32,
  GGUFMetadataValueTypeInt32,
  GGUFMetadataValueTypeFloat32,
  GGUFMetadataValueTypeBool,
  GGUFMetadataValueTypeString,
  GGUFMetadataValueTypeArray,
  GGUFMetadataValueTypeUint64,
  GGUFMetadataValueTypeInt64,
  GGUFMetadataValueTypeFloat64,
  _GGUFMetadataValueTypeCount  // Unknown
};

struct GGUFMetadataKV {
  // Key is the key of the metadata key-value pair,
  // which is no larger than 64 bytes long.
  std::string key;  // Using std::string for dynamic string handling

  // ValueType is the type of the metadata value.
  GGUFMetadataValueType value_type;  // Enum to represent value types

  // Value is the value of the metadata key-value pair.
  std::any value;
};

struct GGUFMetadataKVArrayValue {
  /* Basic */

  // Type is the type of the array item.
  GGUFMetadataValueType type;  // Enum to represent value types

  // Len is the length of the array.
  uint64_t len;  // Using uint64_t for length

  // Array holds all array items.
  std::vector<std::any> arr;
  /* Appendix */

  // start_offset is the offset in bytes of the GGUFMetadataKVArrayValue in the GGUFFile file.
  int64_t start_offset;  // Using int64_t for offset

  // Size is the size of the array in bytes.
  int64_t size;  // Using int64_t for size
};

struct GGUFTensorInfo {
  /* Basic */
  virtual ~GGUFTensorInfo() {}
  // Name is the name of the tensor,
  // which is no larger than 64 bytes long.
  std::string name;
  // NDimensions is the number of dimensions of the tensor.
  uint32_t n_dimensions;
  // Dimensions is the dimensions of the tensor,
  // the length is NDimensions.
  std::vector<uint64_t> dimensions;
  // Type is the type of the tensor.
  GGMLType type;
  // Offset is the offset in bytes of the tensor's data in this file.
  //
  // The offset is relative to tensor data, not to the start of the file.
  uint64_t offset;

  /* Appendix */

  // StartOffset is the offset in bytes of the GGUFTensorInfo in the GGUFFile file.
  //
  // The offset is the start of the file.
  int64_t start_offset;
};

struct GGUFHelper {
  uint8_t* data;
  uint8_t* d_close;
  uint64_t file_size;

  bool OpenAndMMap(const std::string& file_path) {
#ifdef _WIN32
    HANDLE file_handle = INVALID_HANDLE_VALUE;
    HANDLE file_mapping = nullptr;
    file_handle =
        CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle == INVALID_HANDLE_VALUE) {
      std::cout << "Failed to open file" << std::endl;
      return false;
    }
    // Get the file size
    LARGE_INTEGER file_size_struct;
    if (!GetFileSizeEx(file_handle, &file_size_struct)) {
      CloseHandle(file_handle);
      std::cout << "Failed to open file" << std::endl;
      return false;
    }
    file_size = static_cast<size_t>(file_size_struct.QuadPart);

    // Create a file mapping object
    file_mapping =
        CreateFileMappingA(file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (file_mapping == nullptr) {
      CloseHandle(file_handle);
      std::cout << "Failed to create file mapping" << std::endl;
      return false;
    }

    // Map the file into memory
    data = static_cast<uint8_t*>(
        MapViewOfFile(file_mapping, FILE_MAP_READ, 0, 0, file_size));
    if (data == nullptr) {
      CloseHandle(file_mapping);
      CloseHandle(file_handle);
      std::cout << "Failed to map file" << std::endl;
      return false;
    }

    // Close the file handle, as it is no longer needed after mapping
    CloseHandle(file_handle);
    d_close = data;
#else
    file_size = std::filesystem::file_size(file_path);

    int fd = open(file_path.c_str(), O_RDONLY);
    // Memory-map the file
    data = static_cast<uint8_t*>(
        mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (data == MAP_FAILED) {
      perror("Error mapping file");
      close(fd);
      return false;
    }

    close(fd);
    d_close = data;
#endif
    return true;
  }

  ~GGUFHelper() { Close(); }

  void Close() {
#ifdef _WIN32
    if (d_close != nullptr) {
      UnmapViewOfFile(d_close);
      d_close = nullptr;
    }
#else
    if (d_close != nullptr && d_close != MAP_FAILED) {
      munmap(d_close, file_size);
      d_close = nullptr;
    }
#endif
  }

  template <typename T>
  T Read() {
    static_assert(std::is_floating_point<T>::value ||
                  std::is_integral<T>::value || std::is_same<T, bool>::value);
    T res = *reinterpret_cast<const T*>(data);
    data += sizeof(T);
    return res;
  }

  std::string ReadString() {
    auto l = Read<uint64_t>();
    std::string res(reinterpret_cast<const char*>(data), l);
    data += l;
    return res;
  }

  GGUFMetadataKVArrayValue ReadArray() {
    GGUFMetadataKVArrayValue v;
    v.start_offset = (data - d_close);
    auto arr_type = Read<uint32_t>();
    auto arr_length = Read<uint64_t>();
    for (uint64_t i = 0; i < arr_length; ++i) {
      switch (arr_type) {
        case GGUFMetadataValueTypeUint8:
          v.arr.push_back(Read<uint8_t>());
          break;
        case GGUFMetadataValueTypeInt8:
          v.arr.push_back(Read<int8_t>());
          break;
        case GGUFMetadataValueTypeUint16:
          v.arr.push_back(Read<uint16_t>());
          break;
        case GGUFMetadataValueTypeInt16:
          v.arr.push_back(Read<uint16_t>());
          break;
        case GGUFMetadataValueTypeUint32:
          v.arr.push_back(Read<uint32_t>());
          break;
        case GGUFMetadataValueTypeInt32:
          v.arr.push_back(Read<int32_t>());
          break;
        case GGUFMetadataValueTypeFloat32:
          v.arr.push_back(Read<float>());
          break;
        case GGUFMetadataValueTypeBool:
          v.arr.push_back(Read<bool>());
          break;
        case GGUFMetadataValueTypeString:
          v.arr.push_back(ReadString());
          break;
        case GGUFMetadataValueTypeUint64:
          v.arr.push_back(Read<uint64_t>());
          break;
        case GGUFMetadataValueTypeInt64:
          v.arr.push_back(Read<int64_t>());
          break;
        case GGUFMetadataValueTypeFloat64:
          v.arr.push_back(Read<double>());
          break;
        default:
          std::cout << "Invalid type: " << arr_type;
      }
    }
    v.size = data - v.start_offset - d_close - 4 - 8;
    return v;
  }

  std::any ReadValue(GGUFMetadataValueType vt) {
    switch (vt) {
      case GGUFMetadataValueTypeUint8:
        return Read<uint8_t>();
      case GGUFMetadataValueTypeInt8:
        return Read<int8_t>();
      case GGUFMetadataValueTypeUint16:
        return Read<uint16_t>();
      case GGUFMetadataValueTypeInt16:
        return Read<uint16_t>();
      case GGUFMetadataValueTypeUint32:
        return Read<uint32_t>();
      case GGUFMetadataValueTypeInt32:
        return Read<int32_t>();
      case GGUFMetadataValueTypeFloat32:
        return Read<float>();
      case GGUFMetadataValueTypeBool:
        return Read<bool>();
      case GGUFMetadataValueTypeString:
        return ReadString();
      case GGUFMetadataValueTypeArray:
        return ReadArray();
      case GGUFMetadataValueTypeUint64:
        return Read<uint64_t>();
      case GGUFMetadataValueTypeInt64:
        return Read<int64_t>();
      case GGUFMetadataValueTypeFloat64:
        return Read<double>();
      default:
        std::cout << "Invalid type: " << vt;
    }
  }

  GGUFMetadataKV ReadMetadataKV() {
    GGUFMetadataKV kv;
    kv.key = ReadString();
    auto vt = Read<uint32_t>();
    kv.value_type = GGUFMetadataValueType(vt);
    kv.value = ReadValue(kv.value_type);
    return kv;
  }

  GGUFTensorInfo ReadTensorInfo() {
    GGUFTensorInfo ti;
    ti.start_offset = data - d_close;
    ti.name = ReadString();
    ti.n_dimensions = Read<uint32_t>();
    ti.dimensions.resize(ti.n_dimensions);
    for (size_t i = 0; i < ti.n_dimensions; i++) {
      ti.dimensions[i] = Read<uint64_t>();
    }
    auto v = Read<uint32_t>();
    ti.type = GGMLType(v);
    ti.offset = Read<uint64_t>();
    return ti;
  }
};

constexpr const auto ErrGGUFFileInvalidFormat = "invalid GGUF format";

struct GGUFHeader {
  // Magic is a magic number that announces that this is a GGUF file.
  GGUFMagic magic;
  // Version is a version of the GGUF file format.
  GGUFVersion version;
  // TensorCount is the number of tensors in the file.
  uint64_t tensor_count;
  // MetadataKVCount is the number of key-value pairs in the metadata.
  uint64_t metadata_kv_count;
  // MetadataKV are the key-value pairs in the metadata,
  std::vector<GGUFMetadataKV> metadata_kv;

  std::pair<GGUFMetadataKV, bool> Get(const std::string& name) {
    for (auto& kv : metadata_kv) {
      if (kv.key == name) {
        return std::pair(kv, true);
      }
    }
    return std::pair(GGUFMetadataKV{}, false);
  }
};

using GGUFTensorInfos = std::vector<GGUFTensorInfo>;
// using GGUFLayerTensorInfos = std::vector<std::shared_ptr<GGUFTensorInfos>>;
struct GGUFNamedTensorInfos : public GGUFTensorInfo {
  GGUFNamedTensorInfos(const std::string& n) { GGUFTensorInfo::name = n; }
  std::vector<std::shared_ptr<GGUFTensorInfo>> items;
};

struct GGUFFile {
  /* Basic */

  // header is the header of the GGUF file.
  GGUFHeader header;
  // tensor_infos are the tensor infos of the GGUF file,
  // the size of TensorInfos is equal to `Header.TensorCount`.
  std::vector<GGUFTensorInfo> tensor_infos;

  // padding is the padding size of the GGUF file,
  // which is used to split Header and TensorInfos from tensor data.
  int64_t padding;
  // split_paddings holds the padding size slice of the GGUF file splits,
  // each item represents splitting Header and TensorInfos from tensor data.
  //
  // The length of split_paddings is the number of split files.
  std::vector<int64_t> split_paddings;
  // tensor_data_start_offset is the offset in bytes of the tensor data in this file.
  //
  // The offset is the start of the file.
  int64_t tensor_data_start_offset;
  // split_tensor_data_start_offsets holds the offset slice in bytes of the tensor data of the GGUF file splits,
  // each item represents the offset of the tensor data in the split file.
  //
  // The length of split_tensor_data_start_offsets is the number of split files.
  std::vector<int64_t> split_tensor_data_start_offsets;

  /* Appendix */

  // size is the size of the GGUF file,
  // if the file is split, the size is the sum of all split files.
  GGUFBytesScalar size;
  // split_sizes holds the size slice of the GGUF file splits,
  // each item represents the size of the split file.
  //
  // The length of split_sizes is the number of split files.
  std::vector<GGUFBytesScalar> split_sizes;
  // model_size is the size of the model when loading.
  GGUFBytesScalar model_size;
  // split_model_sizes holds the size slice of the model,
  // each item represents a size when loading of the split file.
  //
  // The length of split_model_sizes is the number of split files.
  std::vector<GGUFBytesScalar> split_model_sizes;

  // model_parameters is the number of the model parameters.
  GGUFParametersScalar model_parameters;
  // model_bits_per_weight is the bits per weight of the model,
  // which describes how many bits are used to store a weight,
  // higher is better.
  GGUFBitsPerWeightScalar model_bits_per_weight;
  using GGUFLayerTensorInfos = std::vector<std::shared_ptr<GGUFTensorInfo>>;
  GGUFLayerTensorInfos layers() {
    GGUFLayerTensorInfos ret;
    std::unordered_map<std::string, std::shared_ptr<GGUFTensorInfo>> pm;
    for (size_t i = 0; i < tensor_infos.size(); i++) {
      auto ps = string_utils::SplitBy(tensor_infos[i].name, ".");
      if (ps.size() < 2) {
        ret.push_back(std::make_shared<GGUFTensorInfo>(tensor_infos[i]));
        continue;
      }
      if (ps[0] == "blk" || ps[0] == "mm") {
        auto p = ps[0] + "." + ps[1];
        if (pm.find(p) == pm.end()) {
          auto l = std::make_shared<GGUFNamedTensorInfos>(p);
          pm[p] = l;
          ret.push_back(l);
        }
        auto& l = std::static_pointer_cast<GGUFNamedTensorInfos>(pm[p])->items;
        l.push_back(std::make_shared<GGUFTensorInfo>(tensor_infos[i]));
      } else if (ps[0] == "v" || ps[0] == "t") {  // Clip
        auto p = ps[0];
        if (pm.find(p) == pm.end()) {
          auto xl = std::make_shared<GGUFNamedTensorInfos>(p);
          pm[p] = xl;
          ret.push_back(xl);
        }
        auto& xl = std::static_pointer_cast<GGUFNamedTensorInfos>(pm[p])->items;
        if (ps[1] != "blk" || ps.size() < 3) {
          xl.push_back(std::make_shared<GGUFTensorInfo>(tensor_infos[i]));
          continue;
        }
        p = ps[0] + "." + ps[1] + "." + ps[2];
        if (pm.find(p) == pm.end()) {
          auto l = std::make_shared<GGUFNamedTensorInfos>(p);
          pm[p] = l;
          xl.push_back(l);
        }
        auto& l = std::static_pointer_cast<GGUFNamedTensorInfos>(pm[p])->items;
        l.push_back(std::make_shared<GGUFTensorInfo>(tensor_infos[i]));
      } else if (ps[0] == "decoder" || ps[0] == "encoder") {  // BERT
        auto p = ps[0];
        if (pm.find(p) == pm.end()) {
          auto xl = std::make_shared<GGUFNamedTensorInfos>(p);
          pm[p] = xl;
          ret.push_back(xl);
        }
        auto& xl = std::static_pointer_cast<GGUFNamedTensorInfos>(pm[p])->items;

        if (ps[1] != "block" || ps.size() < 3) {
          xl.push_back(std::make_shared<GGUFTensorInfo>(tensor_infos[i]));
          continue;
        }
        p = ps[0] + "." + ps[1] + "." + ps[2];

        if (pm.find(p) == pm.end()) {
          auto l = std::make_shared<GGUFNamedTensorInfos>(p);
          pm[p] = l;
          xl.push_back(l);
        }
        auto& l = std::static_pointer_cast<GGUFNamedTensorInfos>(pm[p])->items;
        l.push_back(std::make_shared<GGUFTensorInfo>(tensor_infos[i]));
      } else {
        ret.push_back(std::make_shared<GGUFTensorInfo>(tensor_infos[i]));
      }
    }
    return ret;
  }

  struct CutResult {
    GGUFLayerTensorInfos before;
    GGUFLayerTensorInfos after;
    bool found;
  };

  CutResult Cut(const GGUFLayerTensorInfos& ltis,
                const std::vector<std::string>& names) {
    CutResult res;
    std::unordered_set<std::string> ns(names.begin(), names.end());
    for (size_t i = 0; i < ltis.size(); i++) {
      if (auto v = std::dynamic_pointer_cast<GGUFNamedTensorInfos>(ltis[i])) {
        if (ns.find(v->name) != ns.end()) {
          res.before.push_back(v);
          continue;
        }
        res.after.push_back(v);
      } else if (auto v = std::dynamic_pointer_cast<GGUFTensorInfo>(ltis[i])) {
        if (ns.find(v->name) != ns.end()) {
          res.before.push_back(v);
          continue;
        }
        res.after.push_back(v);
      }
    }
    return res;
  }

  std::pair<std::shared_ptr<GGUFTensorInfo>, bool> Get(
      const GGUFLayerTensorInfos& ltis, const std::string& name) {
    for (auto& gi : ltis) {
      if (gi->name == name) {
        return std::pair(gi, true);
      }
    }
    return std::make_pair(nullptr, false);
  }

  GGUFTokenizer Tokenizer() {
    GGUFTokenizer gt;

    const std::string modelKey = "tokenizer.ggml.model";
    const std::string tokensKey = "tokenizer.ggml.tokens";
    const std::string mergesKey = "tokenizer.ggml.merges";
    const std::string addedTokensKey = "tokenizer.ggml.added_tokens";
    const std::string bosTokenIDKey = "tokenizer.ggml.bos_token_id";
    const std::string eosTokenIDKey = "tokenizer.ggml.eos_token_id";
    const std::string eotTokenIDKey = "tokenizer.ggml.eot_token_id";
    const std::string eomTokenIDKey = "tokenizer.ggml.eom_token_id";
    const std::string unknownTokenIDKey = "tokenizer.ggml.unknown_token_id";
    const std::string separatorTokenIDKey = "tokenizer.ggml.separator_token_id";
    const std::string paddingTokenIDKey = "tokenizer.ggml.padding_token_id";

    gt.bos_token_id = -1;
    gt.eos_token_id = -1;
    gt.eot_token_id = -1;
    gt.eom_token_id = -1;
    gt.unknown_token_id = -1;
    gt.separator_token_id = -1;
    gt.padding_token_id = -1;

    if (auto [v, ok] = header.Get(modelKey); ok) {
      assert(v.value_type == GGUFMetadataValueTypeString);
      gt.model = std::any_cast<std::string>(v.value);
    }

    if (auto [v, ok] = header.Get(tokensKey); ok) {
      auto arr = std::any_cast<GGUFMetadataKVArrayValue>(v.value);
      gt.tokens_length = arr.len;
      gt.token_size = arr.size;
    }
    if (auto [v, ok] = header.Get(mergesKey); ok) {
      auto arr = std::any_cast<GGUFMetadataKVArrayValue>(v.value);
      gt.merges_length = arr.len;
      gt.merges_size = arr.size;
    }
    if (auto [v, ok] = header.Get(addedTokensKey); ok) {
      gt.added_tokens_length =
          std::any_cast<GGUFMetadataKVArrayValue>(v.value).len;
    }
    if (auto [v, ok] = header.Get(bosTokenIDKey); ok) {
      gt.bos_token_id = std::any_cast<int64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(eosTokenIDKey); ok) {
      gt.eos_token_id = std::any_cast<int64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(eotTokenIDKey); ok) {
      gt.eot_token_id = std::any_cast<int64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(eomTokenIDKey); ok) {
      gt.eom_token_id = std::any_cast<int64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(unknownTokenIDKey); ok) {
      gt.unknown_token_id = std::any_cast<int64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(separatorTokenIDKey); ok) {
      gt.separator_token_id = std::any_cast<int64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(paddingTokenIDKey); ok) {
      gt.padding_token_id = std::any_cast<int64_t>(v.value);
    }
    return gt;
  }

  GGUFArchitecture clipArchitecture() {
    GGUFArchitecture ga;
    std::string hasTextEncoderKey = "clip.has_text_encoder";
    std::string hasVisionEncoderKey = "clip.has_vision_encoder";
    std::string projectorTypeKey = "clip.projector_type";

    std::string textEmbeddingLengthKey = "clip.text.embedding_length";
    std::string textBlockCountKey = "clip.text.block_count";
    std::string textFeedForwardLengthKey = "clip.text.feed_forward_length";
    std::string textAttentionHeadCountKey = "clip.text.attention.head_count";
    std::string textAttentionLayerNormRMSEpsilonKey =
        "clip.text.attention.layer_norm_epsilon";

    std::string visionEmbeddingLengthKey = "clip.vision.embedding_length";
    std::string visionBlockCountKey = "clip.vision.block_count";
    std::string visionFeedForwardLengthKey = "clip.vision.feed_forward_length";
    std::string visionAttentionHeadCountKey =
        "clip.vision.attention.head_count";
    std::string visionAttentionLayerNormRMSEpsilonKey =
        "clip.vision.attention.layer_norm_epsilon";

    ga.Type = "projector";
    ga.Architecture = "clip";

    if (auto [v, ok] = header.Get(hasTextEncoderKey); ok) {
      ga.ClipHasTextEncoder = std::any_cast<bool>(v.value);
    }
    if (auto [v, ok] = header.Get(hasVisionEncoderKey); ok) {
      ga.ClipHasVisionEncoder = std::any_cast<bool>(v.value);
    }
    if (auto [v, ok] = header.Get(projectorTypeKey); ok) {
      ga.ClipProjectorType = std::any_cast<std::string>(v.value);
    } else {
      ga.ClipProjectorType = "mlp";
    }

    if (auto [v, ok] = header.Get(textEmbeddingLengthKey); ok) {
      ga.EmbeddingLength = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(textBlockCountKey); ok) {
      ga.BlockCount = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(textFeedForwardLengthKey); ok) {
      ga.FeedForwardLength = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(textAttentionHeadCountKey); ok) {
      ga.AttentionHeadCount = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(textAttentionLayerNormRMSEpsilonKey); ok) {
      ga.AttentionLayerNormRMSEpsilon = std::any_cast<float>(v.value);
    }

    if (auto [v, ok] = header.Get(visionEmbeddingLengthKey); ok) {
      ga.EmbeddingLength = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(visionBlockCountKey); ok) {
      ga.BlockCount = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(visionFeedForwardLengthKey); ok) {
      ga.FeedForwardLength = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(visionAttentionHeadCountKey); ok) {
      ga.AttentionHeadCount = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(visionAttentionLayerNormRMSEpsilonKey); ok) {
      ga.AttentionLayerNormRMSEpsilon = std::any_cast<float>(v.value);
    }

    ga.AttentionHeadCountKV = ga.AttentionHeadCount;

    {
      if (ga.AttentionHeadCountKV > 0) {
        ga.EmbeddingGQA = ga.AttentionHeadCount / ga.AttentionHeadCountKV;
      }
      if (ga.AttentionHeadCount > 0) {
        ga.EmbeddingKeyGQA =
            uint64_t(ga.AttentionKeyLength) * ga.AttentionHeadCountKV;
        ga.EmbeddingValueGQA =
            uint64_t(ga.AttentionValueLength) * ga.AttentionHeadCountKV;
      }
      if (ga.Architecture == "mamba") {
        ga.EmbeddingKeyGQA =
            uint64_t((ga.SSMConvolutionKernel - 1) * ga.SSMInnerSize);
        ga.EmbeddingValueGQA = uint64_t(ga.SSMStateSize * ga.SSMInnerSize);
      }
    }

    return ga;
  }

  GGUFArchitecture adapterArchitecture(const std::string& arch) {
    GGUFArchitecture ga;
    const std::string typeKey = "adapter.type";
    const std::string loraAlphaKey = "adapter.lora.alpha";
    const std::string controlVectorLayerCountKey =
        "adapter.control_vector.layer_count";
    const std::string controlVectorLayerCountKey2 =
        "control_vector.layer_count";

    ga.Type = "adapter";
    ga.Architecture = arch;

    if (auto [v, ok] = header.Get(typeKey); ok) {
      ga.AdapterType = std::any_cast<std::string>(v.value);
    }
    if (auto [v, ok] = header.Get(loraAlphaKey); ok) {
      ga.AdapterLoRAAlpha = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(controlVectorLayerCountKey); ok) {
      ga.AdapterControlVectorLayerCount = std::any_cast<uint32_t>(v.value);
    } else if (auto [v, ok] = header.Get(controlVectorLayerCountKey2); ok) {
      ga.AdapterControlVectorLayerCount = std::any_cast<uint32_t>(v.value);
    }

    return ga;
  }

  GGUFArchitecture modelArchitecture(const std::string& arch) {
    GGUFArchitecture ga;

    std::string contextLengthKey = arch + ".context_length";
    std::string embeddingLengthKey = arch + ".embedding_length";
    std::string blockCountKey = arch + ".block_count";
    std::string feedForwardLengthKey = arch + ".feed_forward_length";

    std::string expertFeedForwardLengthKey =
        arch + ".expert_feed_forward_length";
    std::string expertSharedFeedForwardLengthKey =
        arch + ".expert_shared_feed_forward_length";
    std::string expertCountKey = arch + ".expert_count";
    std::string expertUsedCountKey = arch + ".expert_used_count";

    std::string attentionHeadCountKey = arch + ".attention.head_count";
    std::string attentionHeadCountKVKey = arch + ".attention.head_count_kv";
    std::string attentionMaxALiBIBiasKey = arch + ".attention.max_alibi_bias";
    std::string attentionMaxALiBIBiasKey2 = arch + ".attention.alibi_bias_max";
    std::string attentionClampKQVKey = arch + ".attention.clamp_kqv";
    std::string attentionClampKQVKey2 = arch + ".attention.clip_kqv";
    std::string attentionLayerNormEpsilonKey =
        arch + ".attention.layer_norm_epsilon";
    std::string attentionLayerNormRMSEpsilonKey =
        arch + ".attention.layer_norm_rms_epsilon";
    std::string attentionKeyLengthKey = arch + ".attention.key_length";
    std::string attentionValueLengthKey = arch + ".attention.value_length";
    std::string attentionCausalKey = arch + ".attention.causal";

    std::string ropeDimensionCountKey = arch + ".rope.dimension_count";
    std::string ropeFrequencyBaseKey = arch + ".rope.freq_base";
    std::string ropeScaleLinearKey = arch + ".rope.scale_linear";
    std::string ropeScalingTypeKey = arch + ".rope.scaling.type";
    std::string ropeScalingFactorKey = arch + ".rope.scaling.factor";
    std::string ropeScalingOriginalContextKey =
        arch + ".rope.scaling.original_context_length";  // uint32 maybe
    std::string ropeScalingFinetunedKey = arch + ".rope.scaling.finetuned";

    std::string ssmConvolutionKernelKey = arch + ".ssm.conv_kernel";
    std::string ssmInnerSizeKey = arch + ".ssm.inner_size";
    std::string ssmStateSizeKey = arch + ".ssm.state_size";
    std::string ssmTimeStepRankKey = arch + ".ssm.time_step_rank";

    std::string vocabularyLengthKey = arch + ".vocab_size";
    std::string tokenizerGGMLTokensKey = "tokenizer.ggml.tokens";

    ga.Type = "model";
    ga.Architecture = arch;

    if (auto [v, ok] = header.Get(contextLengthKey); ok) {
      ga.MaximumContextLength = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(embeddingLengthKey); ok) {
      ga.EmbeddingLength = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(blockCountKey); ok) {
      ga.BlockCount = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(feedForwardLengthKey); ok) {
      ga.FeedForwardLength = std::any_cast<uint64_t>(v.value);
    }

    if (auto [v, ok] = header.Get(expertCountKey); ok) {
      ga.ExpertCount = std::any_cast<uint32_t>(v.value);
    }
    if (auto [v, ok] = header.Get(expertUsedCountKey); ok) {
      ga.ExpertUsedCount = std::any_cast<uint32_t>(v.value);
    }
    if (auto [v, ok] = header.Get(expertFeedForwardLengthKey); ok) {
      ga.ExpertFeedForwardLength = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(expertSharedFeedForwardLengthKey); ok) {
      ga.ExpertSharedFeedForwardLength = std::any_cast<uint64_t>(v.value);
    }

    if (auto [v, ok] = header.Get(attentionHeadCountKey); ok) {
      ga.AttentionHeadCount = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(attentionHeadCountKVKey); ok) {
      ga.AttentionHeadCountKV = std::any_cast<uint64_t>(v.value);
    } else {
      ga.AttentionHeadCountKV = ga.AttentionHeadCount;
    }
    if (auto [v, ok] = header.Get(attentionMaxALiBIBiasKey); ok) {
      ga.AttentionMaxALiBIBias = std::any_cast<float>(v.value);
    } else if (auto [v, ok] = header.Get(attentionMaxALiBIBiasKey2); ok) {
      ga.AttentionMaxALiBIBias = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(attentionClampKQVKey); ok) {
      ga.AttentionClampKQV = std::any_cast<float>(v.value);
    } else if (auto [v, ok] = header.Get(attentionClampKQVKey2); ok) {
      ga.AttentionClampKQV = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(attentionLayerNormEpsilonKey); ok) {
      ga.AttentionLayerNormEpsilon = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(attentionLayerNormRMSEpsilonKey); ok) {
      ga.AttentionLayerNormRMSEpsilon = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(attentionKeyLengthKey); ok) {
      ga.AttentionKeyLength = std::any_cast<uint32_t>(v.value);
    } else if (ga.AttentionHeadCount != 0) {
      ga.AttentionKeyLength =
          uint32_t(ga.EmbeddingLength / ga.AttentionHeadCount);
    }
    if (auto [v, ok] = header.Get(attentionValueLengthKey); ok) {
      ga.AttentionValueLength = std::any_cast<uint32_t>(v.value);
    } else if (ga.AttentionHeadCount != 0) {
      ga.AttentionValueLength =
          uint32_t(ga.EmbeddingLength / ga.AttentionHeadCount);
    }
    if (auto [v, ok] = header.Get(attentionCausalKey); ok) {
      ga.AttentionCausal = std::any_cast<bool>(v.value);
    } else {
      ga.AttentionCausal = true;
    }

    if (auto [v, ok] = header.Get(ropeDimensionCountKey); ok) {
      ga.RoPEDimensionCount = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(ropeFrequencyBaseKey); ok) {
      ga.RoPEFrequencyBase = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(ropeScaleLinearKey); ok) {
      ga.RoPEScalingType = "linear";
      ga.RoPEScalingFactor = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(ropeScalingTypeKey); ok) {
      ga.RoPEScalingType = std::any_cast<std::string>(v.value);
    }
    if (auto [v, ok] = header.Get(ropeScalingFactorKey); ok) {
      ga.RoPEScalingFactor = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(ropeScalingOriginalContextKey); ok) {
      ga.RoPEScalingOriginalContextLength = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(ropeScalingFinetunedKey); ok) {
      ga.RoPEScalingFinetuned = std::any_cast<bool>(v.value);
    }

    if (auto [v, ok] = header.Get(ssmConvolutionKernelKey); ok) {
      ga.SSMConvolutionKernel = std::any_cast<uint32_t>(v.value);
    }
    if (auto [v, ok] = header.Get(ssmInnerSizeKey); ok) {
      ga.SSMInnerSize = std::any_cast<uint32_t>(v.value);
    }
    if (auto [v, ok] = header.Get(ssmStateSizeKey); ok) {
      ga.SSMStateSize = std::any_cast<uint32_t>(v.value);
    }
    if (auto [v, ok] = header.Get(ssmTimeStepRankKey); ok) {
      ga.SSMTimeStepRank = std::any_cast<uint32_t>(v.value);
    }

    if (auto [v, ok] = header.Get(vocabularyLengthKey); ok) {
      ga.VocabularyLength = std::any_cast<uint64_t>(v.value);
    } else if (auto [v, ok] = header.Get(tokenizerGGMLTokensKey); ok) {
      ga.VocabularyLength =
          std::any_cast<GGUFMetadataKVArrayValue>(v.value).len;
    }

    {
      if (ga.AttentionHeadCountKV > 0) {
        ga.EmbeddingGQA = ga.AttentionHeadCount / ga.AttentionHeadCountKV;
      }
      if (ga.AttentionHeadCount > 0) {
        ga.EmbeddingKeyGQA =
            uint64_t(ga.AttentionKeyLength) * ga.AttentionHeadCountKV;
        ga.EmbeddingValueGQA =
            uint64_t(ga.AttentionValueLength) * ga.AttentionHeadCountKV;
      }
      if (ga.Architecture == "mamba") {
        ga.EmbeddingKeyGQA =
            uint64_t((ga.SSMConvolutionKernel - 1) * ga.SSMInnerSize);
        ga.EmbeddingValueGQA = uint64_t(ga.SSMStateSize * ga.SSMInnerSize);
      }
    }

    return ga;
  }

  GGUFArchitecture Architecture() {
    GGUFArchitecture ga;
    const std::string generalTypeKey = "general.type";
    const std::string generalArchitectureKey = "general.architecture";
    const std::string controlVectorModelHintKey = "controlvector.model_hint";

    std::string typ = "model";
    std::string arch = "llama";

    {
      if (auto [v, ok] = header.Get(generalTypeKey); ok) {
        typ = std::any_cast<std::string>(v.value);
      }
      if (auto [v, ok] = header.Get(generalArchitectureKey); ok) {
        arch = std::any_cast<std::string>(v.value);
      }
    }

    if (arch == "clip") {
      return clipArchitecture();
    } else if (arch == "controlvector") {
      arch = "llama";
      if (auto [v, ok] = header.Get(controlVectorModelHintKey); ok) {
        arch = std::any_cast<std::string>(v.value);
      }
      return adapterArchitecture(arch);
    }
    if (typ == "adapter") {
      return adapterArchitecture(arch);
    }
    return modelArchitecture(arch);
  }
};

GGUFFile ParseGgufFile(const std::string& path) {
  GGUFFile gf;
  GGUFHelper h;
  h.OpenAndMMap(path);

  GGUFMagic magic = h.Read<GGUFMagic>();
  std::cout << "magic: " << magic << std::endl;
  gf.header.magic = magic;
  GGUFVersion version = h.Read<GGUFVersion>();
  auto tensor_count = h.Read<uint64_t>();
  ;
  gf.header.tensor_count += tensor_count;

  auto metadata_kv_count = h.Read<uint64_t>();
  gf.header.metadata_kv_count += metadata_kv_count;

  // metadata kv
  {
    std::vector<GGUFMetadataKV> kvs;
    kvs.resize(metadata_kv_count);
    for (size_t i = 0; i < metadata_kv_count; i++) {
      kvs[i] = h.ReadMetadataKV();
    }
    for (auto& kv : kvs) {
      if (kv.key == "split.no") {
        gf.header.metadata_kv_count--;
        continue;
      }
      gf.header.metadata_kv.push_back(kv);
    }
  }

  // tensor infos
  // if(gf.tensor_infos.empty()) {
  //   auto [tc, ok] = gf.header.Get("split.tensors.count");
  //   if(ok) {
  //     gf.tensor_infos.resize(std::any_cast<int>(tc.value));
  //   } else {
  //     gf.tensor_infos.resize(tensor_count);
  //   }
  // }
  {
    std::vector<GGUFTensorInfo> tis;
    tis.resize(tensor_count);
    for (size_t i = 0; i < tensor_count; i++) {
      tis[i] = h.ReadTensorInfo();
    }
    gf.tensor_infos = tis;
  }

  int64_t pds = h.data - h.d_close;
  int64_t padding;
  uint32_t ag = 32;
  if (auto [v, ok] = gf.header.Get("general.alignment"); ok) {
    ag = std::any_cast<uint32_t>(v.value);
  }
  padding = int64_t(ag) - (pds % int64_t(ag));
  gf.padding = padding;
  gf.split_paddings.push_back(padding);

  // tensor data offset
  auto tensor_data_offset = pds + padding;
  gf.tensor_data_start_offset = tensor_data_offset;
  gf.split_tensor_data_start_offsets.push_back(tensor_data_offset);

  // size
  auto size = GGUFBytesScalar(h.file_size);
  gf.size += size;
  gf.split_sizes.push_back(size);

  // model size
  auto model_size = GGUFBytesScalar(h.file_size - tensor_data_offset);
  gf.model_size += model_size;
  gf.split_model_sizes.push_back(model_size);
}
}  // namespace hardware