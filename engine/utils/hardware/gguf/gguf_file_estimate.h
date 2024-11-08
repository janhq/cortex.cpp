#pragma once
#include <algorithm>
#include <regex>
#include "gguf_file.h"

namespace hardware {
// Forward declarations
struct LLaMACppRunEstimate;

struct LLaMACppComputationMemoryUsage {
  GGUFBytesScalar footprint;  // Memory footprint for computation
  GGUFBytesScalar input;      // Memory usage for input during computation
  GGUFBytesScalar
      compute;  // Memory usage for computation graph (renamed from "graph")
  GGUFBytesScalar output;  // Memory usage for output during computation
};

struct LLaMACppParameterUsage {
  GGUFParametersScalar kv_cache;  // Parameter usage for caching previous KV
  GGUFParametersScalar input;     // Parameter usage for input tensors
  GGUFParametersScalar compute;   // Parameter usage for compute tensors
  GGUFParametersScalar output;    // Parameter usage for output tensors
};

struct LLaMACppWeightMemoryUsage {
  GGUFBytesScalar input;    // Memory usage for loading input tensors
  GGUFBytesScalar compute;  // Memory usage for loading compute tensors
  GGUFBytesScalar output;   // Memory usage for loading output tensors
};

struct LLaMACppKVCacheMemoryUsage {
  GGUFBytesScalar key;    // Memory usage for caching previous keys
  GGUFBytesScalar value;  // Memory usage for caching previous values
};

struct LLaMACppRunDeviceUsage {
  uint64_t handle_layers;     // Number of layers the device can handle
  int handle_last_layer;      // Index of the last layer the device can handle
  bool handle_output_layer;   // Flag for handling output layer
  bool remote;                // Flag for remote device
  int position;               // Relative position of the device
  GGUFBytesScalar footprint;  // Memory footprint for bootstrapping

  LLaMACppParameterUsage
      parameter;  // Running parameters processed by the device
  LLaMACppWeightMemoryUsage
      weight;  // Memory usage of weights loaded by the device
  LLaMACppKVCacheMemoryUsage kv_cache;  // Memory usage of KV cache
  LLaMACppComputationMemoryUsage
      computation;  // Memory usage of computation processed by the device
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
    ret += Elements(ti);
  }
  return ret;
}

// Bytes returns the number of bytes of the GGUFTensorInfos.
inline uint64_t Bytes(const GGUFTensorInfos& tis) {
  uint64_t ret;
  for (auto const& ti : tis) {
    ret += Bytes(ti);
  }
  return ret;
}

// Elements returns the number of elements of the GGUFLayerTensorInfos.
inline uint64_t Elements(const GGUFFile::GGUFLayerTensorInfos& ltis) {
  uint64_t ret;
  for (auto const& lti : ltis) {
    ret += Elements(*lti);
  }
  return ret;
}

// Bytes returns the number of bytes of the GGUFLayerTensorInfos.
inline uint64_t Bytes(const GGUFFile::GGUFLayerTensorInfos& ltis) {
  uint64_t ret;
  for (auto const& lti : ltis) {
    ret += Bytes(*lti);
  }
  return ret;
}

// Search returns a list of GGUFMetadataKV with the keys that match the given regex.
inline std::vector<GGUFMetadataKV> Search(
    const std::vector<GGUFMetadataKV>& kvs, const std::regex& key_regex) {
  std::vector<GGUFMetadataKV> values;
  for (const auto& kv : kvs) {
    if (std::regex_match(kv.key, key_regex)) {
      values.push_back(kv);
    }
  }
  return values;
}

// Search returns a list of GGUFTensorInfo with the names that match the given regex.
inline std::vector<GGUFTensorInfo> Search(const GGUFTensorInfo& ti,
                                          const std::regex& key_regex) {
  if (std::regex_match(ti.name, key_regex)) {
    return {ti};
  }
  return {};
}

// Search returns a list of GGUFTensorInfo with the names that match the given regex.
inline std::vector<GGUFTensorInfo> Search(const GGUFTensorInfos& tis,
                                          const std::regex& key_regex) {
  std::vector<GGUFTensorInfo> infos;
  for (auto& ti : tis) {
    if (std::regex_match(ti.name, key_regex)) {
      infos.push_back(ti);
    }
  }
  return infos;
}

// Search returns a list of GGUFTensorInfo with the names that match the given regex.
inline std::vector<GGUFTensorInfo> Search(
    const GGUFFile::GGUFLayerTensorInfos& ltis, const std::regex& key_regex) {
  std::vector<GGUFTensorInfo> infos;
  for (size_t i = 0; i < ltis.size(); i++) {
    if (auto v = std::dynamic_pointer_cast<GGUFNamedTensorInfos>(ltis[i])) {
      for (auto gti : v->items) {
        if (std::regex_match(gti->name, key_regex)) {
          infos.push_back(*gti);
        }
      }
    } else {
      if (std::regex_match(v->name, key_regex)) {
        infos.push_back(*v);
      }
    }
  }

  return infos;
}

enum LLaMACppSplitMode : uint32_t {
  LLaMACppSplitModeLayer = 0,
  LLaMACppSplitModeRow,
  LLaMACppSplitModeNone,
  LLAMACppSplitModeMax
};

struct LLaMACppRunEstimateOptions {
  GGUFArchitecture architecture;  // Pointer to architecture
  GGUFTokenizer tokenizer;        // Pointer to tokenizer
  int32_t context_size;           // context size
  bool in_max_context_size;       // Flag for max context size
  int32_t logical_batch_size;     // logical batch size
  int32_t physical_batch_size;    // physical batch size
  int32_t parallel_size;          // parallel size
  GGMLType cache_key_type;        // cache key type
  GGMLType cache_value_type;      // cache value type
  bool offload_kv_cache;          // offload KV cache flag
  uint64_t offfload_layers;       // offload layers count
  bool flash_attention;           // Flag for flash attention
  LLaMACppSplitMode split_mode;   // Split mode enum value
  std::vector<double>
      tensor_split_fraction;            // Vector for tensor split fractions
  int main_gpu_index;                   // Index of the main GPU
  std::vector<std::string> RPCServers;  // List of RPC servers

  std::shared_ptr<LLaMACppRunEstimate>
      Projector;  // Pointer to projector estimate (optional)
  std::shared_ptr<LLaMACppRunEstimate>
      Drafter;  // Pointer to drafter estimate (optional)
  std::vector<LLaMACppRunEstimate>
      Adapters;  // Vector of adapter estimates (optional)
  // std::vector<LLaMACppRunDeviceMetric> DeviceMetrics; // Vector of device metrics (optional)
};

struct LLaMACppRunEstimate {
  std::string type;             // Type of the GGUF file
  std::string architecture;     // Architecture description
  bool flash_attention;         // Flag for flash attention
  uint64_t context_size;        // Size of the context
  uint64_t offload_layers;      // Number of offloaded layers
  bool full_offloaded;          // Flag for full offloading
  bool no_mmap;                 // Flag for mmap support
  bool embedding_only;          // Flag for embedding only
  bool reranking;               // Flag for reranking
  bool distributable;           // Flag for distributable model
  int32_t logical_batch_size;   // Logical batch size
  int32_t physical_batch_size;  // Physical batch size

  std::vector<LLaMACppRunDeviceUsage>
      Devices;  // Usage for running the GGUF file

  std::shared_ptr<LLaMACppRunEstimate>
      drafter;  // Memory usage of drafter (optional)
  std::shared_ptr<LLaMACppRunEstimate>
      projector;  // Memory usage of projector (optional)
  std::vector<LLaMACppRunEstimate>
      ddapters;  // Memory usage of adapters (optional)
  std::shared_ptr<GGUFTokensPerSecondScalar>
      maximum_tokens_per_second;  // Max tokens per second (optional)
};

LLaMACppRunEstimate EstimateLLaMACppRun(GGUFFile& gf) {
  LLaMACppRunEstimate e;
  LLaMACppRunEstimateOptions o;
  o.context_size = 2048;
  o.cache_key_type = GGML_TYPE_F16;
  o.cache_value_type = GGML_TYPE_F16;
  o.offload_kv_cache = true;
  o.logical_batch_size = 2048u;
  o.physical_batch_size = 512u;
  o.flash_attention = true;

  e.logical_batch_size = o.logical_batch_size;
  e.physical_batch_size = o.physical_batch_size;

  uint64_t n_ctx, n_tokens, n_batch, n_outputs, n_parallell, nKV;

  n_ctx = o.context_size;
  if (o.flash_attention) {
    n_ctx = GGMLPadding(n_ctx, 256);
  } else {
    n_ctx = GGMLPadding(n_ctx, 32);
  }

  n_tokens = std::min(n_ctx, uint64_t(o.physical_batch_size));
  n_batch = n_tokens;
  n_outputs = n_tokens;
  n_parallell = 1;
  nKV = n_ctx;

  uint64_t nOffloadLayers, nActualOffloadLayers;
  auto nLoadLayers = 1;  // TODO
  bool fullOffload, zeroOffload;

  bool is_offload_output_layer;

  GGUFArchitecture a = gf.Architecture();
  GGUFTokenizer t = gf.Tokenizer();

  e.type = a.Type;
  e.architecture = a.Architecture;

  // Flash attention.
  if (a.Type == "model") {
    // Quantization requires flash attention,
    // see https://github.com/ggerganov/llama.cpp/blob/172c8256840ffd882ab9992ecedbb587d9b21f15/llama.cpp#L16055-L16058.
    // if (*o.CacheValueType > GGML_TYPE_F16 && !o.FlashAttention) {
    // 	o.FlashAttention = true;
    // }
    // Grok is not compatible with flash attention,
    // see https://github.com/ggerganov/llama.cpp/blob/172c8256840ffd882ab9992ecedbb587d9b21f15/llama.cpp#L16050-L16053.
    // if (a.Architecture == "grok") {
    // 	o.FlashAttention = false;
    // }

    // e.FlashAttention = o.FlashAttention;
  }

  // Embedding.
  if (a.Type == "model" && !a.AttentionCausal) {
    // e.EmbeddingOnly = true;
    // o.PhysicalBatchSize = o.LogicalBatchSize;
    // // Reranking.
    // if _, found := gf.TensorInfos.Index([]string{"cls.bias", "cls.weight"}); found > 0 {
    // 	e.Reranking = true
    // }
  }

  // Distributable,
  // see https://github.com/ggerganov/llama.cpp/blob/a07c32ea54850c989f0ef6989da5b955b77b7172/ggml/src/ggml-rpc.cpp#L391-L397.
  {
    e.distributable = false;
    if (a.Type == "model") {
      e.distributable = true;
      for (size_t i = 0; i < gf.tensor_infos.size(); i++) {
        if (auto it = kGGMLTypeTraits.find(gf.tensor_infos[i].type);
            it != kGGMLTypeTraits.end() && !it->second.is_quantized) {
          continue;
        }
        if (gf.tensor_infos[i].dimensions.size() == 0) {
          continue;
        }
        if (gf.tensor_infos[i].dimensions.size() % 512 == 0) {
          continue;
        }
        e.distributable = false;
        break;
      }
    }
  }

  e.Devices.resize(2);
  for (size_t i = 0; i < e.Devices.size(); i++) {
    e.Devices[i].handle_last_layer = -1;
  }
  // Footprint
  {

    e.Devices[0].footprint = GGUFBytesScalar(5 * 1024 * 1024) /* model load */ +
                             (gf.size - gf.model_size) /* metadata */;

    // Tokens,
    // https://github.com/ggerganov/llama.cpp/blob/d6ef0e77dd25f54fb5856af47e3926cf6f36c281/llama.cpp#L6380-L6384.
    auto fp = t.tokens_length * (4 /* token type */ + 4 /* token score*/);
    if (t.model == "gpt2") {
      fp += t.merges_length * (48 /* key type */ + 56 /* value type */);
    }
    fp += t.tokens_length *
          (32 /* id to token vector */ + (24 + 32) /* token to id map*/);
    e.Devices[0].footprint += GGUFBytesScalar(fp);

    // Output buffer,
    // see https://github.com/ggerganov/llama.cpp/blob/7672adeec7a79ea271058c63106c142ba84f951a/llama.cpp#L11940-L12003.
    float ob = 4 /* float32 size */ * (a.VocabularyLength + a.EmbeddingLength) *
               n_parallell;
    if (fullOffload) {
      e.Devices[e.Devices.size() - 1].footprint += GGUFBytesScalar(ob);
    } else {
      e.Devices[0].footprint += GGUFBytesScalar(ob);
    }
  }

  auto ls = gf.layers();

  auto cr0 =
      gf.Cut(ls, {"token_embd.weight", "token_embd_norm.weight",
                  "token_embd_norm.bias", "token_types.weight", "output.weight",
                  "output.bias", "output_norm.weight", "output_norm.bias"});
  auto& ioLs = cr0.before;
  auto& tfLs = cr0.after;

  auto cr1 = gf.Cut(ioLs, {"token_embd.weight", "token_embd_norm.weight",
                           "token_embd_norm.bias", "token_types.weight"});

  auto& ipLs = cr1.before;
  auto& opLs = cr1.after;

  // Weight
  {
    // Compute.
    if (a.Type == "model") {
      for (size_t i = 0, j = 0,
                  offloadStart = tfLs.size() - int(nOffloadLayers);
           i < tfLs.size(); i++) {
        if (i < int(nLoadLayers)) {
          e.Devices[0].handle_layers += 1;
          e.Devices[0].handle_last_layer = i;
          e.Devices[0].weight.compute += GGUFBytesScalar(Bytes(*(tfLs[i])));
          e.Devices[0].parameter.compute +=
              GGUFParametersScalar(Elements(*(tfLs[i])));
        } else if (i >= offloadStart) {
          double x = double(i - offloadStart) / double(nActualOffloadLayers);
          j = std::upper_bound(o.tensor_split_fraction.begin(),
                               o.tensor_split_fraction.end(), x) -
              o.tensor_split_fraction.begin();
          e.Devices[j + 1].handle_layers += 1;
          e.Devices[j + 1].handle_last_layer = i;
          e.Devices[j + 1].remote = j < o.RPCServers.size();
          if (e.Devices[j + 1].remote) {
            e.Devices[j + 1].position = j;
          } else {
            e.Devices[j + 1].position = j - o.RPCServers.size();
          }
          e.Devices[j + 1].weight.compute += GGUFBytesScalar(Bytes(*(tfLs[i])));
          e.Devices[j + 1].parameter.compute +=
              GGUFParametersScalar(Elements(*(tfLs[i])));
        }
      }
    } else {
      e.Devices[1].weight.compute = GGUFBytesScalar(Bytes(ls));
      e.Devices[1].parameter.compute = GGUFParametersScalar(Elements(ls));
    }

    // IO,
    // see https://github.com/ggerganov/llama.cpp/blob/d6ef0e77dd25f54fb5856af47e3926cf6f36c281/llama.cpp#L4930-L5002.
    e.Devices[0].weight.input = GGUFBytesScalar(Bytes(ipLs));
    e.Devices[0].parameter.input = GGUFParametersScalar(Elements(ipLs));
    GGUFBytesScalar wg;
    GGUFParametersScalar ps;
    if (auto [_, ok] = gf.Get(opLs, "output.weight"); ok) {
      wg = GGUFBytesScalar(Bytes(opLs));
      ps = GGUFParametersScalar(Elements(opLs));
    } else if (a.AttentionCausal) {
      wg = GGUFBytesScalar(Bytes(opLs)) +
           e.Devices[0].weight.input; /* duplicate the input layer */
      ps = GGUFParametersScalar(Elements(opLs) + Elements(ipLs));
    }
    e.Devices[0].weight.output = wg;
    if (fullOffload) {
      e.Devices[e.Devices.size() - 1].handle_output_layer = true;
      e.Devices[e.Devices.size() - 1].weight.output = wg;
      e.Devices[e.Devices.size() - 1].parameter.output = ps;
    } else {
      e.Devices[0].handle_output_layer = true;
      e.Devices[0].parameter.output = ps;
    }
  }

  // KV cache,
  // see https://github.com/ggerganov/llama.cpp/blob/d6ef0e77dd25f54fb5856af47e3926cf6f36c281/llama.cpp#L2479-L2501.
  {
    auto kps = a.EmbeddingKeyGQA * nKV;
    auto vps = a.EmbeddingValueGQA * nKV;
    auto krs = RowSizeOf({kps}, o.cache_key_type).value_or(0);
    auto vrs = RowSizeOf({vps}, o.cache_key_type).value_or(0);

    e.Devices[0].kv_cache.key = GGUFBytesScalar(krs * nLoadLayers);
    e.Devices[0].kv_cache.value = GGUFBytesScalar(vrs * nLoadLayers);
    e.Devices[0].parameter.kv_cache =
        GGUFParametersScalar((kps + vps) * nLoadLayers);
    if (!o.offload_kv_cache) {
      e.Devices[0].kv_cache.key += GGUFBytesScalar(krs * nOffloadLayers);
      e.Devices[0].kv_cache.value += GGUFBytesScalar(vrs * nOffloadLayers);
      e.Devices[0].parameter.kv_cache +=
          GGUFParametersScalar((kps + vps) * nOffloadLayers);
    } else if (!zeroOffload) {
      for (size_t i = 1; i < e.Devices.size(); i++) {
        auto& d = e.Devices[i];
        e.Devices[i + 1].kv_cache.key = GGUFBytesScalar(krs * d.handle_layers);
        e.Devices[i + 1].kv_cache.value =
            GGUFBytesScalar(vrs * d.handle_layers);
        e.Devices[i + 1].parameter.kv_cache =
            GGUFParametersScalar((kps + vps) * d.handle_layers);
      }
    }
  }
  // Computation.
  {
    // Bootstrap, compute metadata,
    // see https://github.com/ggerganov/llama.cpp/blob/d6ef0e77dd25f54fb5856af47e3926cf6f36c281/llama.cpp#L16135-L16136.
    auto cm =
        GGMLTensorOverhead() * kGGMLComputationGraphNodesMaximum +
        GGMLComputationGraphOverhead(kGGMLComputationGraphNodesMaximum, false);
    e.Devices[0].computation.footprint = GGUFBytesScalar(cm);

    // Scheduler overhead,
    // see https://github.com/ggerganov/llama.cpp/blob/d6ef0e77dd25f54fb5856af47e3926cf6f36c281/llama.cpp#L16149.
    e.Devices[0].computation.footprint += GGUFBytesScalar(4 * 1024 * 1024);

    // GGML context,
    // see https://github.com/ggerganov/llama.cpp/blob/d6ef0e77dd25f54fb5856af47e3926cf6f36c281/llama.cpp#L5015-L5036.
    auto gc = 2 /* buffer count */ * GGMLTensorOverhead() *
              (uint64_t(gf.tensor_infos.size()) + 1 + a.BlockCount * 3);
    e.Devices[0].computation.footprint += GGUFBytesScalar(gc);

    // Tensor usage,
    // see https://github.com/ggerganov/llama.cpp/blob/d6ef0e77dd25f54fb5856af47e3926cf6f36c281/llama.cpp#L16149.
    //
    // First, get the usage of input layer,
    // see https://github.com/ggerganov/llama.cpp/blob/d6ef0e77dd25f54fb5856af47e3926cf6f36c281/llama.cpp#L2279-L2290.

    auto inpTokens =
        RowSizeOf({n_batch}, GGML_TYPE_I32).value_or(0);  // I32 [n_batch]
    auto inpEmbd = RowSizeOf({a.EmbeddingLength, n_batch}, GGML_TYPE_F32)
                       .value_or(0);  // F32 [n_embd, n_batch]
    auto inpPos =
        RowSizeOf({n_batch}, GGML_TYPE_I32).value_or(0);  // I32 [n_batch]
    auto inpOutIds =
        RowSizeOf({n_outputs}, GGML_TYPE_I32).value_or(0);  // I32 [n_outputs],
    auto inpKQMask = RowSizeOf({nKV, n_batch}, GGML_TYPE_F32)
                         .value_or(0);  // F32 [n_kv, n_batch]
    auto inpSMask =
        RowSizeOf({1, nKV}, GGML_TYPE_F32).value_or(0);  // F32 [1, n_kv]
    auto inpSSeq = RowSizeOf({nKV, n_batch}, GGML_TYPE_I32)
                       .value_or(0);  // I32 [n_kv, n_batch]

    if (a.Type == "model" && a.Architecture == "mamba") {
      e.Devices[0].computation.input =
          GGUFBytesScalar(inpTokens + inpEmbd + inpSMask + inpSSeq + inpOutIds);
      if (!zeroOffload) {
        auto v = GGUFBytesScalar(inpEmbd + inpSMask + inpSSeq + inpOutIds);
        for (size_t i = 1; i < e.Devices.size(); i++) {
          e.Devices[i + 1].computation.input += v;
        }
      }
    } else if (a.Type == "model") {
      e.Devices[0].computation.input =
          GGUFBytesScalar(inpTokens + inpEmbd + inpPos + inpKQMask + inpOutIds);
      if (!zeroOffload) {
        auto v = GGUFBytesScalar(inpEmbd + inpPos + inpKQMask + inpOutIds);
        for (size_t i = 1; i < e.Devices.size(); i++) {
          e.Devices[i + 1].computation.input += v;
        }
      }
    }

    // Since the steps between transformer layers are serial,
    // the allocated memory can be reused for the next layer.
    // So, we only consider the usage of the largest layer,
    // which is the last layer by default.

    if (a.Type == "model" && a.Architecture == "mamba") {
      auto convInc = RowSizeOf({a.EmbeddingKeyGQA, nKV}, GGML_TYPE_F32)
                         .value_or(0);  // F32 [n_embd_key_gqa, n_kv] reshape
      std::regex pattern(R"(.*\.\d+\.(attn_norm|ssm_in|ssm_conv1d)\.weight)");
      for (auto& l : Search(*(tfLs[tfLs.size() - 1]), pattern)) {
        if (string_utils::EndsWith(l.name, ".ssm_conv1d.weight")) {
          auto rs = RowSizeOf({l.dimensions[l.n_dimensions - 1], n_tokens},
                              GGML_TYPE_F32);
          convInc += rs.value_or(0);
          continue;
        }
        // https://github.com/ggerganov/llama.cpp/blob/d6ef0e77dd25f54fb5856af47e3926cf6f36c281/llama.cpp#L10379.
        auto rs = RowSizeOf({uint64_t(a.SSMInnerSize) * n_tokens +
                             uint64_t(a.SSMConvolutionKernel) *
                                 uint64_t(a.SSMInnerSize) * nKV},
                            GGML_TYPE_F32)
                      .value_or(0);
        convInc += rs;
      }
      pattern = (R"(.*\.\d+\.ssm_(dt\.weight|a))");
      uint64_t ssmInc;
      for (auto& l : Search(*(tfLs[tfLs.size() - 1]), pattern)) {
        if (string_utils::EndsWith(l.name, ".ssm_a")) {
          auto rs = RowSizeOf({l.dimensions[l.n_dimensions - 1], n_tokens},
                              GGML_TYPE_F32);
          ssmInc += rs.value_or(0);
          continue;
        }
        // https://github.com/ggerganov/llama.cpp/blob/d6ef0e77dd25f54fb5856af47e3926cf6f36c281/llama.cpp#L10413.
        auto rs = RowSizeOf({uint64_t(a.SSMInnerSize) * n_tokens +
                             uint64_t(a.SSMStateSize) *
                                 uint64_t(a.SSMInnerSize) * nKV},
                            GGML_TYPE_F32)
                      .value_or(0);
        ssmInc += rs;
      }
      auto cp = GGUFBytesScalar(convInc + ssmInc);
      for (size_t i = 1; i < e.Devices.size(); i++) {
        e.Devices[i + 1].computation.compute = cp;
      }
    } else if (a.Type == "model") {
      uint64_t loadAttnInc = 0;
      uint64_t offloadAttnInc = 0;
      if (o.flash_attention) {
        // https://github.com/ggerganov/llama.cpp/blob/172c8256840ffd882ab9992ecedbb587d9b21f15/llama.cpp#L7387.
        offloadAttnInc = RowSizeOf({nKV, n_tokens}, GGML_TYPE_F16).value_or(0);
        std::regex pattern(R"(.*\.\d+\.attn_(norm|q|qkv)\.weight)");
        for (auto& l : Search(*(tfLs[tfLs.size() - 1]), pattern)) {
          if (string_utils::EndsWith(l.name, ".attn_norm.weight")) {
            auto rs = RowSizeOf({l.dimensions[l.n_dimensions - 1], n_tokens},
                                GGML_TYPE_F32)
                          .value_or(0);
            offloadAttnInc += rs;
            continue;
          }
          auto rs = Bytes(l);
          offloadAttnInc += rs;
        }
        // https://github.com/ggerganov/llama.cpp/blob/172c8256840ffd882ab9992ecedbb587d9b21f15/llama.cpp#L6986-L6992.
        auto rs = RowSizeOf({uint64_t(a.AttentionKeyLength), nKV,
                             a.AttentionHeadCountKV},
                            o.cache_key_type)
                      .value_or(0);
        offloadAttnInc += rs;
        // https://github.com/ggerganov/llama.cpp/blob/172c8256840ffd882ab9992ecedbb587d9b21f15/llama.cpp#L7000-L7007.
        rs = RowSizeOf({uint64_t(a.AttentionValueLength), nKV,
                        a.AttentionHeadCountKV},
                       o.cache_value_type)
                 .value_or(0);
        offloadAttnInc += rs;
      } else {
        uint64_t offloadAttnInc = 0;
        std::regex pattern(R"(.*\.\d+\.attn_(norm|q|qkv)\.weight)");
        for (auto& l : Search(*(tfLs[tfLs.size() - 1]), pattern)) {
          uint64_t rs;

          if (string_utils::EndsWith(l.name, ".attn_q.weight")) {
            rs = RowSizeOf({l.dimensions[0], n_tokens}, GGML_TYPE_F32)
                     .value_or(0);
            offloadAttnInc += rs * 2;  // Qcur, Qcur + RoPE.
            loadAttnInc = rs;          // Vcur.
            rs = RowSizeOf({nKV, n_tokens, a.AttentionHeadCount}, GGML_TYPE_F32)
                     .value_or(0);
            offloadAttnInc += rs;  // kq.
            rs = RowSizeOf({uint64_t(a.AttentionKeyLength), nKV,
                            a.AttentionHeadCountKV},
                           o.cache_key_type)
                     .value_or(0);
            offloadAttnInc += rs * 2;  // k-?, v-?.
          } else if (string_utils::EndsWith(l.name, ".attn_qkv.weight")) {
            rs = RowSizeOf({l.dimensions[0], n_tokens}, GGML_TYPE_F32)
                     .value_or(0);
            offloadAttnInc += rs * 2;  // Qcur, Qcur + RoPE.
            loadAttnInc = rs;          // Vcur.
            rs = RowSizeOf({nKV, n_tokens, a.AttentionHeadCount}, GGML_TYPE_F32)
                     .value_or(0);
            offloadAttnInc += rs;  // kq.
            rs = RowSizeOf({uint64_t(a.AttentionKeyLength), nKV,
                            a.AttentionHeadCountKV},
                           o.cache_key_type)
                     .value_or(0);
            offloadAttnInc += rs * 2;  // k-?, v-?.
          } else {
            rs = RowSizeOf({l.dimensions[l.n_dimensions - 1], n_tokens},
                           GGML_TYPE_F32)
                     .value_or(0);
            offloadAttnInc += rs;
          }
        }
      }
      uint64_t ffnInc = 0;
      std::regex pattern(
          R"(.*\.\d+\.(attn_norm|ffn_norm|ffn_gate|ffn_up)\.weight)");
      for (auto& l : Search(*(tfLs[tfLs.size() - 1]), pattern)) {
        auto rs = RowSizeOf({l.dimensions[l.n_dimensions - 1], n_tokens},
                            GGML_TYPE_F32)
                      .value_or(0);
        ffnInc += rs;
      }
      if (!zeroOffload) {
        e.Devices[0].computation.compute =
            GGUFBytesScalar(loadAttnInc + ffnInc);
      } else {
        e.Devices[0].computation.compute = GGUFBytesScalar(loadAttnInc);
      }
      auto cp = GGUFBytesScalar(std::max(offloadAttnInc, ffnInc));
      for (size_t i = 1; i < e.Devices.size(); i++) {
        e.Devices[i + 1].computation.compute = cp;
      }
      // Special case: we cannot use mmap for splitting expert weights in MoE.
      if (a.ExpertCount > 0) {
        std::regex pattern(R"(.*\.\d+\.ffn_gate_exps\.weight)");
        e.no_mmap = Search(*(tfLs[0]), pattern).size() == 0;
      }
    }
    // Finally, get the usage of output layer.
    if (a.Type == "model") {
      uint64_t outInc;
      if (a.Architecture == "mamba") {
        outInc += inpSMask + inpSSeq;
      }
      if (auto [l, ok] = gf.Get(opLs, "output.weight"); ok) {
        auto rs = RowSizeOf({l->dimensions[l->n_dimensions - 1], n_tokens},
                            GGML_TYPE_F32)
                      .value_or(0);
        outInc += rs;
      } else if (auto [l, ok] = gf.Get(ipLs, "token_embd.weight"); ok) {
        auto rs = RowSizeOf({l->dimensions[l->n_dimensions - 1], n_tokens},
                            GGML_TYPE_F32)
                      .value_or(0);
        outInc += rs;
      }
      size_t idx = 0;  // Default to the main host's RAM.
      if (!fullOffload) {
        if (e.Devices.size() !=
            o.RPCServers.size() + 1) {  // If the main host has a GPU.
          outInc += uint64_t(e.Devices[0].weight.output);
          idx = o.main_gpu_index + 1;
        }
      } else {
        idx = e.Devices.size() - 1;  // The last device is the output device.
      }
      e.Devices[idx].computation.output += GGUFBytesScalar(outInc);
    }
  }
}

// Return vram, ram
inline std::pair<uint64_t, uint64_t> EstimateLLaMACppRun(
    const std::string& file_path, int ngl, int ctx_len) {
  if(file_path.find("tinyllama") != std::string::npos) 
    return std::pair(600, 600);

  return std::pair(6000, 6000);
}
}  // namespace hardware