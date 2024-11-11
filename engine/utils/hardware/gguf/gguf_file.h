#pragma once
#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <algorithm>
#include <any>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

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

#define GGUF_LOG(msg)                                                  \
  do {                                                                 \
    std::cout << __FILE__ << "(@" << __LINE__ << "): " << msg << '\n'; \
  } while (false)

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

  // type is the type of the array item.
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

inline std::string to_string(GGUFMetadataValueType vt, const std::any& v) {
  switch (vt) {
    case GGUFMetadataValueTypeUint8:
      return std::to_string(std::any_cast<uint8_t>(v));
    case GGUFMetadataValueTypeInt8:
      return std::to_string(std::any_cast<int8_t>(v));
    case GGUFMetadataValueTypeUint16:
      return std::to_string(std::any_cast<uint16_t>(v));
    case GGUFMetadataValueTypeInt16:
      return std::to_string(std::any_cast<int16_t>(v));
    case GGUFMetadataValueTypeUint32:
      return std::to_string(std::any_cast<uint32_t>(v));
    case GGUFMetadataValueTypeInt32:
      return std::to_string(std::any_cast<int32_t>(v));
    case GGUFMetadataValueTypeFloat32:
      return std::to_string(std::any_cast<float>(v));
    case GGUFMetadataValueTypeBool:
      return std::to_string(std::any_cast<bool>(v));
    case GGUFMetadataValueTypeString:
      return std::any_cast<std::string>(v);
    case GGUFMetadataValueTypeUint64:
      return std::to_string(std::any_cast<uint64_t>(v));
    case GGUFMetadataValueTypeInt64:
      return std::to_string(std::any_cast<int64_t>(v));
    case GGUFMetadataValueTypeFloat64:
      return std::to_string(std::any_cast<double>(v));
    default:
      break;
  }
  return "array";
}
inline std::string to_string(const GGUFMetadataKVArrayValue& arr_v) {
  std::string res;
  auto num = std::min(size_t(5), arr_v.arr.size());
  for (size_t i = 0; i < num; i++) {
    res += to_string(arr_v.type, arr_v.arr[i]) + " ";
  }
  return res;
}

inline std::string to_string(const GGUFMetadataKV& kv) {
  switch (kv.value_type) {
    case GGUFMetadataValueTypeUint8:
      return std::to_string(std::any_cast<uint8_t>(kv.value));
    case GGUFMetadataValueTypeInt8:
      return std::to_string(std::any_cast<int8_t>(kv.value));
    case GGUFMetadataValueTypeUint16:
      return std::to_string(std::any_cast<uint16_t>(kv.value));
    case GGUFMetadataValueTypeInt16:
      return std::to_string(std::any_cast<int16_t>(kv.value));
    case GGUFMetadataValueTypeUint32:
      return std::to_string(std::any_cast<uint32_t>(kv.value));
    case GGUFMetadataValueTypeInt32:
      return std::to_string(std::any_cast<int32_t>(kv.value));
    case GGUFMetadataValueTypeFloat32:
      return std::to_string(std::any_cast<float>(kv.value));
    case GGUFMetadataValueTypeBool:
      return std::to_string(std::any_cast<bool>(kv.value));
    case GGUFMetadataValueTypeString:
      return std::any_cast<std::string>(kv.value);
    case GGUFMetadataValueTypeUint64:
      return std::to_string(std::any_cast<uint64_t>(kv.value));
    case GGUFMetadataValueTypeInt64:
      return std::to_string(std::any_cast<int64_t>(kv.value));
    case GGUFMetadataValueTypeFloat64:
      return std::to_string(std::any_cast<double>(kv.value));
    case GGUFMetadataValueTypeArray:
      return to_string(std::any_cast<GGUFMetadataKVArrayValue>(kv.value));
    default:
      break;
  }
  return "Invalid type ";
}

struct GGUFTensorInfoI {
  virtual ~GGUFTensorInfoI() {}
  // Name is the name of the tensor,
  // which is no larger than 64 bytes long.
  std::string name;

  virtual uint64_t Elements() = 0;
  virtual uint64_t Bytes() = 0;
};

struct GGUFTensorInfo : public GGUFTensorInfoI {
  /* Basic */

  // NDimensions is the number of dimensions of the tensor.
  uint32_t n_dimensions;
  // Dimensions is the dimensions of the tensor,
  // the length is NDimensions.
  std::vector<uint64_t> dimensions;
  // type is the type of the tensor.
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

  uint64_t Elements() {
    if (n_dimensions == 0) {
      return 0;
    }

    uint64_t ret = 1;
    for (size_t i = 0; i < n_dimensions; i++) {
      ret *= dimensions[i];
    }
    return ret;
  }

  uint64_t Bytes() {
    if (n_dimensions == 0) {
      return 0;
    }

    if (kGGMLTypeTraits.find(type) == kGGMLTypeTraits.end()) {
      std::cout << "Invalid type: " << type << std::endl;
      assert(false);
    }

    auto& tt = kGGMLTypeTraits.at(type);

    std::vector<uint64_t> nb(n_dimensions);
    nb[0] = tt.type_size;
    nb[1] = nb[0] * (dimensions[0] / tt.block_size);
    for (size_t i = 2; i < n_dimensions; i++) {
      nb[i] = nb[i - 1] * dimensions[i - 1];
    }

    uint64_t ret;

    if (tt.block_size == 1) {
      ret = tt.type_size;
      for (size_t i = 0; i < n_dimensions; i++) {
        ret += (dimensions[i] - 1) * nb[1];
      }
      return ret;
    }

    ret = dimensions[0] * nb[0] / tt.block_size;
    for (size_t i = 1; i < n_dimensions; i++) {
      ret += (dimensions[i] - 1) * nb[i];
    }
    return ret;
  }
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
    auto r = res;
    data += l;
    return r;
  }

  GGUFMetadataKVArrayValue ReadArray() {
    GGUFMetadataKVArrayValue v;
    v.start_offset = (data - d_close);
    v.type = static_cast<GGUFMetadataValueType>(Read<uint32_t>());
    auto arr_length = Read<uint64_t>();
    for (uint64_t i = 0; i < arr_length; ++i) {
      switch (v.type) {
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
          std::cout << "Invalid type: " << std::to_string(v.type);
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

  std::shared_ptr<GGUFTensorInfo> ReadTensorInfo() {
    auto ti = std::make_shared<GGUFTensorInfo>();
    ti->start_offset = data - d_close;
    ti->name = ReadString();
    ti->n_dimensions = Read<uint32_t>();
    ti->dimensions.resize(ti->n_dimensions);
    for (size_t i = 0; i < ti->n_dimensions; i++) {
      ti->dimensions[i] = Read<uint64_t>();
    }
    auto v = Read<uint32_t>();
    ti->type = GGMLType(v);
    ti->offset = Read<uint64_t>();
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
    for (auto const& kv : metadata_kv) {
      if (kv.key == name) {
        return std::pair(kv, true);
      }
    }
    return std::pair(GGUFMetadataKV{}, false);
  }
};

using GGUFTensorInfos = std::vector<std::shared_ptr<GGUFTensorInfo>>;
// using GGUFLayerTensorInfos = std::vector<std::shared_ptr<GGUFTensorInfos>>;
struct GGUFNamedTensorInfos : public GGUFTensorInfoI {
  GGUFNamedTensorInfos(const std::string& n) { GGUFTensorInfoI::name = n; }
  std::vector<std::shared_ptr<GGUFTensorInfoI>> items;
  uint64_t Elements() {
    uint64_t ret;
    for (auto const& i : items) {
      ret += i->Elements();
    }
    return ret;
  }

  uint64_t Bytes() {
    uint64_t ret;
    for (auto const& i : items) {
      ret += i->Bytes();
    }
    return ret;
  }
};

struct GGUFFile {
  /* Basic */

  // header is the header of the GGUF file.
  GGUFHeader header;
  // tensor_infos are the tensor infos of the GGUF file,
  // the size of TensorInfos is equal to `Header.TensorCount`.
  std::vector<std::shared_ptr<GGUFTensorInfo>> tensor_infos;

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
  using GGUFLayerTensorInfos = std::vector<std::shared_ptr<GGUFTensorInfoI>>;
  GGUFLayerTensorInfos layers() {
    GGUFLayerTensorInfos ret;
    std::unordered_map<std::string, std::shared_ptr<GGUFTensorInfoI>> pm;
    for (size_t i = 0; i < tensor_infos.size(); i++) {
      auto ps = string_utils::SplitBy(tensor_infos[i]->name, ".");
      if (ps.size() < 2) {
        ret.push_back(tensor_infos[i]);
        // GGUF_LOG("GGUFTensorInfo type: " << ret.back()->type);
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

        l.push_back(tensor_infos[i]);
        // GGUF_LOG("type: " << l.back()->type << " ltype: " << pm[p]->type);
      } else if (ps[0] == "v" || ps[0] == "t") {  // Clip
        auto p = ps[0];
        if (pm.find(p) == pm.end()) {
          auto xl = std::make_shared<GGUFNamedTensorInfos>(p);
          pm[p] = xl;
          ret.push_back(xl);
        }
        auto& xl = std::static_pointer_cast<GGUFNamedTensorInfos>(pm[p])->items;
        if (ps[1] != "blk" || ps.size() < 3) {
          xl.push_back(tensor_infos[i]);
          continue;
        }
        p = ps[0] + "." + ps[1] + "." + ps[2];
        if (pm.find(p) == pm.end()) {
          auto l = std::make_shared<GGUFNamedTensorInfos>(p);
          pm[p] = l;
          xl.push_back(l);
        }
        auto& l = std::static_pointer_cast<GGUFNamedTensorInfos>(pm[p])->items;
        l.push_back(tensor_infos[i]);
      } else if (ps[0] == "decoder" || ps[0] == "encoder") {  // BERT
        auto p = ps[0];
        if (pm.find(p) == pm.end()) {
          auto xl = std::make_shared<GGUFNamedTensorInfos>(p);
          pm[p] = xl;
          ret.push_back(xl);
        }
        auto& xl = std::static_pointer_cast<GGUFNamedTensorInfos>(pm[p])->items;

        if (ps[1] != "block" || ps.size() < 3) {
          xl.push_back(tensor_infos[i]);
          continue;
        }
        p = ps[0] + "." + ps[1] + "." + ps[2];

        if (pm.find(p) == pm.end()) {
          auto l = std::make_shared<GGUFNamedTensorInfos>(p);
          pm[p] = l;
          xl.push_back(l);
        }
        auto& l = std::static_pointer_cast<GGUFNamedTensorInfos>(pm[p])->items;
        l.push_back(tensor_infos[i]);
      } else {
        ret.push_back(tensor_infos[i]);
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
        // GGUF_LOG("sangnv");
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
      const std::vector<GGUFTensorInfo>& ltis, const std::string& name) {
    for (auto const& gi : ltis) {
      if (gi.name == name) {
        return std::pair(std::make_shared<GGUFTensorInfo>(gi), true);
      }
    }
    return std::make_pair(nullptr, false);
  }

  // Get returns the IGGUFTensorInfos with the given name,
  // and true if found, and false otherwise.
  std::pair<std::shared_ptr<GGUFTensorInfo>, bool> Get(
      const GGUFLayerTensorInfos& ltis, const std::string& name) {
    for (auto &lti : ltis) {
      if (auto v = std::dynamic_pointer_cast<GGUFNamedTensorInfos>(lti)) {
        auto [info, found] = Get(v->items, name);
        if (found)
          return std::pair(info, found);
      } else {
        auto s = std::static_pointer_cast<GGUFTensorInfo>(lti);
        if (s->name == name) {
          return std::pair(s, true);
        }
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
      gt.bos_token_id = std::stoll(to_string(v));
    }
    if (auto [v, ok] = header.Get(eosTokenIDKey); ok) {
      gt.eos_token_id = std::stoll(to_string(v));
    }
    if (auto [v, ok] = header.Get(eotTokenIDKey); ok) {
      gt.eot_token_id = std::stoll(to_string(v));
    }
    if (auto [v, ok] = header.Get(eomTokenIDKey); ok) {
      gt.eom_token_id = std::stoll(to_string(v));
    }
    if (auto [v, ok] = header.Get(unknownTokenIDKey); ok) {
      gt.unknown_token_id = std::stoll(to_string(v));
    }
    if (auto [v, ok] = header.Get(separatorTokenIDKey); ok) {
      gt.separator_token_id = std::stoll(to_string(v));
    }
    if (auto [v, ok] = header.Get(paddingTokenIDKey); ok) {
      gt.padding_token_id = std::stoll(to_string(v));
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

    ga.type = "projector";
    ga.architecture = "clip";

    if (auto [v, ok] = header.Get(hasTextEncoderKey); ok) {
      ga.clip_has_text_encoder = std::any_cast<bool>(v.value);
    }
    if (auto [v, ok] = header.Get(hasVisionEncoderKey); ok) {
      ga.clip_has_vision_encoder = std::any_cast<bool>(v.value);
    }
    if (auto [v, ok] = header.Get(projectorTypeKey); ok) {
      ga.clip_projector_type = std::any_cast<std::string>(v.value);
    } else {
      ga.clip_projector_type = "mlp";
    }

    if (auto [v, ok] = header.Get(textEmbeddingLengthKey); ok) {
      ga.embedding_length = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(textBlockCountKey); ok) {
      ga.block_count = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(textFeedForwardLengthKey); ok) {
      ga.feed_forward_length = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(textAttentionHeadCountKey); ok) {
      ga.attention_head_count = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(textAttentionLayerNormRMSEpsilonKey); ok) {
      ga.attention_layer_norm_rms_epsilon = std::any_cast<float>(v.value);
    }

    if (auto [v, ok] = header.Get(visionEmbeddingLengthKey); ok) {
      ga.embedding_length = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(visionBlockCountKey); ok) {
      ga.block_count = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(visionFeedForwardLengthKey); ok) {
      ga.feed_forward_length = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(visionAttentionHeadCountKey); ok) {
      ga.attention_head_count = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(visionAttentionLayerNormRMSEpsilonKey); ok) {
      ga.attention_layer_norm_rms_epsilon = std::any_cast<float>(v.value);
    }

    ga.attention_head_count_kv = ga.attention_head_count;

    {
      if (ga.attention_head_count_kv > 0) {
        ga.embedding_gqa = ga.attention_head_count / ga.attention_head_count_kv;
      }
      if (ga.attention_head_count > 0) {
        ga.embedding_key_gqa =
            uint64_t(ga.attention_key_length) * ga.attention_head_count_kv;
        ga.embedding_value_gqa =
            uint64_t(ga.attention_value_length) * ga.attention_head_count_kv;
      }
      if (ga.architecture == "mamba") {
        ga.embedding_key_gqa =
            uint64_t((ga.ssm_convolution_kernel - 1) * ga.ssm_inner_size);
        ga.embedding_value_gqa = uint64_t(ga.ssm_state_size * ga.ssm_inner_size);
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

    ga.type = "adapter";
    ga.architecture = arch;

    if (auto [v, ok] = header.Get(typeKey); ok) {
      ga.adapter_type = std::any_cast<std::string>(v.value);
    }
    if (auto [v, ok] = header.Get(loraAlphaKey); ok) {
      ga.adapter_lora_alpha = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(controlVectorLayerCountKey); ok) {
      ga.adapter_control_vector_layer_count = std::any_cast<uint32_t>(v.value);
    } else if (auto [v, ok] = header.Get(controlVectorLayerCountKey2); ok) {
      ga.adapter_control_vector_layer_count = std::any_cast<uint32_t>(v.value);
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

    ga.type = "model";
    ga.architecture = arch;

    if (auto [v, ok] = header.Get(contextLengthKey); ok) {
      ga.max_context_length = std::stoull(to_string(v));
    }
    if (auto [v, ok] = header.Get(embeddingLengthKey); ok) {
      ga.embedding_length = std::stoull(to_string(v));
    }
    if (auto [v, ok] = header.Get(blockCountKey); ok) {
      ga.block_count = std::stoull(to_string(v));
    }
    if (auto [v, ok] = header.Get(feedForwardLengthKey); ok) {
      ga.feed_forward_length = std::stoull(to_string(v));
    }

    if (auto [v, ok] = header.Get(expertCountKey); ok) {
      ga.expert_count = std::any_cast<uint32_t>(v.value);
    }
    if (auto [v, ok] = header.Get(expertUsedCountKey); ok) {
      ga.expert_used_count = std::any_cast<uint32_t>(v.value);
    }
    if (auto [v, ok] = header.Get(expertFeedForwardLengthKey); ok) {
      ga.expert_feed_forward_length = std::any_cast<uint64_t>(v.value);
    }
    if (auto [v, ok] = header.Get(expertSharedFeedForwardLengthKey); ok) {
      ga.expert_shared_feed_forward_length = std::any_cast<uint64_t>(v.value);
    }

    if (auto [v, ok] = header.Get(attentionHeadCountKey); ok) {
      ga.attention_head_count = std::stoull(to_string(v));
    }
    if (auto [v, ok] = header.Get(attentionHeadCountKVKey); ok) {
      ga.attention_head_count_kv = std::stoull(to_string(v));
    } else {
      ga.attention_head_count_kv = ga.attention_head_count;
    }
    if (auto [v, ok] = header.Get(attentionMaxALiBIBiasKey); ok) {
      ga.attention_max_alibi_bias = std::stof(to_string(v));
    } else if (auto [v, ok] = header.Get(attentionMaxALiBIBiasKey2); ok) {
      ga.attention_max_alibi_bias = std::stof(to_string(v));
    }
    if (auto [v, ok] = header.Get(attentionClampKQVKey); ok) {
      ga.attention_clamp_kqv = std::any_cast<float>(v.value);
    } else if (auto [v, ok] = header.Get(attentionClampKQVKey2); ok) {
      ga.attention_clamp_kqv = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(attentionLayerNormEpsilonKey); ok) {
      ga.attention_layer_norm_epsilon = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(attentionLayerNormRMSEpsilonKey); ok) {
      ga.attention_layer_norm_rms_epsilon = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(attentionKeyLengthKey); ok) {
      ga.attention_key_length = std::stoul(to_string(v));
    } else if (ga.attention_head_count != 0) {
      ga.attention_key_length =
          uint32_t(ga.embedding_length / ga.attention_head_count);
    }
    if (auto [v, ok] = header.Get(attentionValueLengthKey); ok) {
      ga.attention_value_length = std::stoul(to_string(v));
    } else if (ga.attention_head_count != 0) {
      ga.attention_value_length =
          uint32_t(ga.embedding_length / ga.attention_head_count);
    }
    if (auto [v, ok] = header.Get(attentionCausalKey); ok) {
      ga.attention_causal = std::any_cast<bool>(v.value);
    } else {
      ga.attention_causal = true;
    }

    if (auto [v, ok] = header.Get(ropeDimensionCountKey); ok) {
      ga.rope_dimension_count = std::stoull(to_string(v));
    }
    if (auto [v, ok] = header.Get(ropeFrequencyBaseKey); ok) {
      ga.rope_frequency_base = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(ropeScaleLinearKey); ok) {
      ga.rope_scaling_type = "linear";
      ga.rope_scaling_factor = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(ropeScalingTypeKey); ok) {
      ga.rope_scaling_type = std::any_cast<std::string>(v.value);
    }
    if (auto [v, ok] = header.Get(ropeScalingFactorKey); ok) {
      ga.rope_scaling_factor = std::any_cast<float>(v.value);
    }
    if (auto [v, ok] = header.Get(ropeScalingOriginalContextKey); ok) {
      ga.rope_scaling_original_context_length = std::stoull(to_string(v));
    }
    if (auto [v, ok] = header.Get(ropeScalingFinetunedKey); ok) {
      ga.rope_scaling_finetuned = std::any_cast<bool>(v.value);
    }

    if (auto [v, ok] = header.Get(ssmConvolutionKernelKey); ok) {
      ga.ssm_convolution_kernel = std::stoul(to_string(v));
    }
    if (auto [v, ok] = header.Get(ssmInnerSizeKey); ok) {
      ga.ssm_inner_size = std::stoul(to_string(v));
    }
    if (auto [v, ok] = header.Get(ssmStateSizeKey); ok) {
      ga.ssm_state_size = std::stoul(to_string(v));
    }
    if (auto [v, ok] = header.Get(ssmTimeStepRankKey); ok) {
      ga.ssm_time_step_rank = std::stoul(to_string(v));
    }

    if (auto [v, ok] = header.Get(vocabularyLengthKey); ok) {
      ga.vocabulary_length = std::stoull(to_string(v));
    } else if (auto [v, ok] = header.Get(tokenizerGGMLTokensKey); ok) {
      ga.vocabulary_length =
          std::any_cast<GGUFMetadataKVArrayValue>(v.value).len;
    }

    {
      if (ga.attention_head_count_kv > 0) {
        ga.embedding_gqa = ga.attention_head_count / ga.attention_head_count_kv;
      }
      if (ga.attention_head_count > 0) {
        ga.embedding_key_gqa =
            uint64_t(ga.attention_key_length) * ga.attention_head_count_kv;
        ga.embedding_value_gqa =
            uint64_t(ga.attention_value_length) * ga.attention_head_count_kv;
      }
      if (ga.architecture == "mamba") {
        ga.embedding_key_gqa =
            uint64_t((ga.ssm_convolution_kernel - 1) * ga.ssm_inner_size);
        ga.embedding_value_gqa = uint64_t(ga.ssm_state_size * ga.ssm_inner_size);
      }
    }

    return ga;
  }

  GGUFArchitecture architecture() {
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

// Elements returns the number of elements of the GGUFTensorInfo,
// which is inspired by
// https://github.com/ggerganov/ggml/blob/a10a8b880c059b3b29356eb9a9f8df72f03cdb6a/src/ggml.c#L2597-L2601.
inline uint64_t Elements(const GGUFTensorInfo& ti) {
  if (ti.n_dimensions == 0) {
    return 0;
  }

  uint64_t ret = 1;
  for (size_t i = 0; i < ti.n_dimensions; i++) {
    ret *= ti.dimensions[i];
  }
  return ret;
}

// Bytes returns the number of bytes of the GGUFTensorInfo,
// which is inspired by
// https://github.com/ggerganov/ggml/blob/a10a8b880c059b3b29356eb9a9f8df72f03cdb6a/src/ggml.c#L2609-L2626.
inline uint64_t Bytes(const GGUFTensorInfo& ti) {
  if (ti.n_dimensions == 0) {
    return 0;
  }

  if (kGGMLTypeTraits.find(ti.type) == kGGMLTypeTraits.end()) {
    std::cout << "Invalid type: " << ti.type << std::endl;
    assert(false);
  }

  auto& tt = kGGMLTypeTraits.at(ti.type);

  std::vector<uint64_t> nb(ti.n_dimensions);
  nb[0] = tt.type_size;
  nb[1] = nb[0] * (ti.dimensions[0] / tt.block_size);
  for (size_t i = 2; i < ti.n_dimensions; i++) {
    nb[i] = nb[i - 1] * ti.dimensions[i - 1];
  }

  uint64_t ret;

  if (tt.block_size == 1) {
    ret = tt.type_size;
    for (size_t i = 0; i < ti.n_dimensions; i++) {
      ret += (ti.dimensions[i] - 1) * nb[1];
    }
    return ret;
  }

  ret = ti.dimensions[0] * nb[0] / tt.block_size;
  for (size_t i = 1; i < ti.n_dimensions; i++) {
    ret += (ti.dimensions[i] - 1) * nb[i];
  }
  return ret;
}

// Count returns the number of GGUF tensors of the GGUFTensorInfo,
// which is always 1.
inline uint64_t Count(GGUFTensorInfo& ti) {
  return 1;
}

// Elements returns the number of elements of the GGUFTensorInfos.
inline uint64_t Elements(const GGUFTensorInfos& tis) {
  uint64_t ret;
  for (auto const& ti : tis) {
    ret += Elements(*ti);
  }
  return ret;
}

// Bytes returns the number of bytes of the GGUFTensorInfos.
inline uint64_t Bytes(const GGUFTensorInfos& tis) {
  uint64_t ret;
  for (auto const& ti : tis) {
    ret += Bytes(*ti);
  }
  return ret;
}

// Elements returns the number of elements of the GGUFLayerTensorInfos.
inline uint64_t Elements(const GGUFFile::GGUFLayerTensorInfos& ltis) {
  uint64_t ret;
  for (auto const& lti : ltis) {
    ret += lti->Elements();
  }
  return ret;
}

// Bytes returns the number of bytes of the GGUFLayerTensorInfos.
inline uint64_t Bytes(const GGUFFile::GGUFLayerTensorInfos& ltis) {
  uint64_t ret;
  for (auto const& lti : ltis) {
    ret += lti->Bytes();
  }
  return ret;
}

inline GGUFFile ParseGgufFile(const std::string& path) {
  GGUFFile gf;
  GGUFHelper h;
  h.OpenAndMMap(path);

  GGUFMagic magic = h.Read<GGUFMagic>();
  // GGUF_LOG("magic: " << magic);
  gf.header.magic = magic;
  GGUFVersion version = h.Read<GGUFVersion>();
  auto tensor_count = h.Read<uint64_t>();
  // GGUF_LOG("tensor_count: " << tensor_count);
  gf.header.tensor_count += tensor_count;

  auto metadata_kv_count = h.Read<uint64_t>();
  gf.header.metadata_kv_count += metadata_kv_count;
  // GGUF_LOG("metadata_kv_count: " << metadata_kv_count);

  // metadata kv
  {
    std::vector<GGUFMetadataKV> kvs;
    kvs.resize(metadata_kv_count);
    for (size_t i = 0; i < metadata_kv_count; i++) {
      kvs[i] = h.ReadMetadataKV();
      // GGUF_LOG("i: " << i << " " << kvs[i].value_type << " " << kvs[i].key
      //                << ": " << to_string(kvs[i]));
    }
    for (auto const& kv : kvs) {
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
    std::vector<std::shared_ptr<GGUFTensorInfo>> tis;
    tis.resize(tensor_count);
    for (size_t i = 0; i < tensor_count; i++) {
      tis[i] = h.ReadTensorInfo();
      // auto tto_string = [](const std::vector<size_t>& ds) -> std::string {
      //   std::string res = "[";
      //   for (auto d : ds)
      //     res += std::to_string(d) + " ";
      //   return res + "]";
      // };
      // auto ds = tto_string(tis[i]->dimensions);
      // GGUF_LOG("i: " << i << " name: " << tis[i]->name
      //                << " type: " << to_string(tis[i]->type) << " dimensions: "
      //                << std::to_string(tis[i]->n_dimensions) << " " << ds);
    }
    gf.tensor_infos = tis;
  }

  int64_t pds = h.data - h.d_close;
  int64_t padding;
  // The global alignment to use, as described above.
  // This can vary to allow for different alignment schemes, but it must be a multiple of 8.
  // Some writers may not write the alignment.
  // If the alignment is not specified, assume it is 32.
  uint32_t ag = 32;
  if (auto [v, ok] = gf.header.Get("general.alignment"); ok) {
    ag = std::any_cast<uint32_t>(v.value);
  }
  padding = int64_t(ag) - (pds % int64_t(ag));
  // GGUF_LOG("pds: " << pds << ", padding: " << padding);
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

  // model parameters
  gf.model_parameters = GGUFParametersScalar(Elements(gf.tensor_infos));
  // GGUF_LOG("model_parameters: " << gf.model_parameters);

  // bpw
  if (gf.model_parameters != 0) {
    gf.model_bits_per_weight = GGUFBitsPerWeightScalar(
        double(gf.model_size) * 8 / double(gf.model_parameters));
    // GGUF_LOG("model_bits_per_weight: " << gf.model_bits_per_weight);
  }
  return gf;
}
}  // namespace hardware