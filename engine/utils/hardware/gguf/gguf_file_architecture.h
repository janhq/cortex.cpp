#pragma once
#include <iostream>
#include <string>
#include <optional>
#include <cstdint>

namespace hardware {
// GGUFArchitecture struct
struct GGUFArchitecture {
    /* Basic */

    // Type describes the type of the file, default is "model".
    std::string Type; // Type of the file
    // Architecture describes what architecture this model implements.
    std::string Architecture; // Model architecture
    // MaximumContextLength(n_ctx_train) is the maximum context length of the model.
    uint64_t MaximumContextLength; // Maximum context length
    // EmbeddingLength(n_embd) is the length of the embedding layer.
    uint64_t EmbeddingLength; // Length of embedding layer
    // BlockCount(n_layer) is the number of blocks of attention and feed-forward layers.
    uint64_t BlockCount; // Number of blocks
    // FeedForwardLength(n_ff) is the length of the feed-forward layer.
    uint64_t FeedForwardLength; // Length of feed-forward layer
    // ExpertFeedForwardLength(expert_feed_forward_length) is the length of the feed-forward layer in the expert model.
    uint64_t ExpertFeedForwardLength; // Length in expert model
    // ExpertSharedFeedForwardLength(expert_shared_feed_forward_length) is the length of shared feed-forward layer in expert model.
    uint64_t ExpertSharedFeedForwardLength; // Length of shared feed-forward layer
    // ExpertCount(n_expert) is the number of experts in MoE models.
    uint32_t ExpertCount; // Number of experts
    // ExpertUsedCount(n_expert_used) is the number of experts used during evaluation in MoE models.
    uint32_t ExpertUsedCount; // Number of experts used
    // AttentionHeadCount(n_head) is the number of attention heads.
    uint64_t AttentionHeadCount; // Number of attention heads
    // AttentionHeadCountKV(n_head_kv) is the number of attention heads per group used in Grouped-Query-Attention.
    uint64_t AttentionHeadCountKV; // Attention heads per group
    // AttentionMaxALiBIBias is the maximum bias to use for ALiBI.
    float AttentionMaxALiBIBias; // Maximum ALiBI bias
    // AttentionClampKQV describes a value `C`, which is used to clamp Q, K, V tensors between `[-C, C]`.
    float AttentionClampKQV; // Clamping value for Q, K, V tensors
    // AttentionLayerNormEpsilon is the epsilon value used in LayerNorm.
    float AttentionLayerNormEpsilon; // Epsilon for LayerNorm
    // AttentionLayerNormRMSEpsilon is the epsilon value used in RMSNorm.
    float AttentionLayerNormRMSEpsilon; // Epsilon for RMSNorm
    // AttentionKeyLength(n_embd_head_k) is the size of a key head.
    uint32_t AttentionKeyLength; // Size of key head
    // AttentionValueLength(n_embd_head_v) is the size of a value head.
    uint32_t AttentionValueLength; // Size of value head
    // AttentionCausal indicates if attention is causal.
    bool AttentionCausal; // Causal attention flag
    // RoPEDimensionCount is number of dimensions in RoPE (Rotary Positional Encoding).
    uint64_t RoPEDimensionCount; // Dimensions in RoPE
    // RoPEFrequencyBase is base frequency for RoPE.
    float RoPEFrequencyBase; // Base frequency for RoPE
    // RoPEFrequencyScale is frequency scale for RoPE.
    std::string RoPEScalingType;  // Scaling type for RoPE
    float RoPEScalingFactor;  // Scaling factor for RoPE
    uint64_t RoPEScalingOriginalContextLength;  // Original context length for RoPE scaling
    bool RoPEScalingFinetuned;  // Indicates if RoPE scaling is fine-tuned
    uint32_t SSMConvolutionKernel;  // Size of convolution kernel in SSM (Selective State Space Model)
    uint32_t SSMInnerSize;  // Embedding size in SSM state
    uint32_t SSMStateSize;  // Size of recurrent state in SSM
    uint32_t SSMTimeStepRank;  // Rank of time steps in SSM
    uint64_t VocabularyLength;  // Size of vocabulary

   /* Appendix */

   uint64_t EmbeddingGQA;  // GQA for embedding layer
   uint64_t EmbeddingKeyGQA;  // Number of key GQA in embedding layer
   uint64_t EmbeddingValueGQA;  // Number of value GQA in embedding layer

   /* Clip Model Options */
   bool ClipHasTextEncoder;  // Indicates if clip model has text encoder
   bool ClipHasVisionEncoder;  // Indicates if clip model has vision encoder
   std::string ClipProjectorType;  // Type of projector used in clip model

   /* Adapter Options */
   std::string AdapterType;  // Type of adapter used
   float AdapterLoRAAlpha;  // Alpha value for LoRA adapter 
   uint32_t AdapterControlVectorLayerCount;  // Layers in control vector (only for control_vector architecture)
};
}