#pragma once
#include <iostream>
#include <string>
#include <optional>
#include <cstdint>

namespace hardware {
// GGUFArchitecture struct
struct GGUFArchitecture {
    /* Basic */

    // type describes the type of the file, default is "model".
    std::string type; // type of the file
    // architecture describes what architecture this model implements.
    std::string architecture; // Model architecture
    // max_context_length(n_ctx_train) is the maximum context length of the model.
    uint64_t max_context_length; // Maximum context length
    // embedding_length(n_embd) is the length of the embedding layer.
    uint64_t embedding_length; // Length of embedding layer
    // block_count(n_layer) is the number of blocks of attention and feed-forward layers.
    uint64_t block_count; // Number of blocks
    // feed_forward_length(n_ff) is the length of the feed-forward layer.
    uint64_t feed_forward_length; // Length of feed-forward layer
    // expert_feed_forward_length(expert_feed_forward_length) is the length of the feed-forward layer in the expert model.
    uint64_t expert_feed_forward_length; // Length in expert model
    // expert_shared_feed_forward_length(expert_shared_feed_forward_length) is the length of shared feed-forward layer in expert model.
    uint64_t expert_shared_feed_forward_length; // Length of shared feed-forward layer
    // expert_count(n_expert) is the number of experts in MoE models.
    uint32_t expert_count; // Number of experts
    // expert_used_count(n_expert_used) is the number of experts used during evaluation in MoE models.
    uint32_t expert_used_count; // Number of experts used
    // attention_head_count(n_head) is the number of attention heads.
    uint64_t attention_head_count; // Number of attention heads
    // attention_head_count_kv(n_head_kv) is the number of attention heads per group used in Grouped-Query-Attention.
    uint64_t attention_head_count_kv; // Attention heads per group
    // attention_max_alibi_bias is the maximum bias to use for ALiBI.
    float attention_max_alibi_bias; // Maximum ALiBI bias
    // attention_clamp_kqv describes a value `C`, which is used to clamp Q, K, V tensors between `[-C, C]`.
    float attention_clamp_kqv; // Clamping value for Q, K, V tensors
    // attention_layer_norm_epsilon is the epsilon value used in LayerNorm.
    float attention_layer_norm_epsilon; // Epsilon for LayerNorm
    // attention_layer_norm_rms_epsilon is the epsilon value used in RMSNorm.
    float attention_layer_norm_rms_epsilon; // Epsilon for RMSNorm
    // attention_key_length(n_embd_head_k) is the size of a key head.
    uint32_t attention_key_length; // Size of key head
    // attention_value_length(n_embd_head_v) is the size of a value head.
    uint32_t attention_value_length; // Size of value head
    // attention_causal indicates if attention is causal.
    bool attention_causal; // Causal attention flag
    // rope_dimension_count is number of dimensions in RoPE (Rotary Positional Encoding).
    uint64_t rope_dimension_count; // Dimensions in RoPE
    // rope_frequency_base is base frequency for RoPE.
    float rope_frequency_base; // Base frequency for RoPE
    // RoPEFrequencyScale is frequency scale for RoPE.
    std::string rope_scaling_type;  // Scaling type for RoPE
    float rope_scaling_factor;  // Scaling factor for RoPE
    uint64_t rope_scaling_original_context_length;  // Original context length for RoPE scaling
    bool rope_scaling_finetuned;  // Indicates if RoPE scaling is fine-tuned
    uint32_t ssm_convolution_kernel;  // Size of convolution kernel in SSM (Selective State Space Model)
    uint32_t ssm_inner_size;  // Embedding size in SSM state
    uint32_t ssm_state_size;  // Size of recurrent state in SSM
    uint32_t ssm_time_step_rank;  // Rank of time steps in SSM
    uint64_t vocabulary_length;  // Size of vocabulary

   /* Appendix */

   uint64_t embedding_gqa;  // GQA for embedding layer
   uint64_t embedding_key_gqa;  // Number of key GQA in embedding layer
   uint64_t embedding_value_gqa;  // Number of value GQA in embedding layer

   /* Clip Model Options */
   bool clip_has_text_encoder;  // Indicates if clip model has text encoder
   bool clip_has_vision_encoder;  // Indicates if clip model has vision encoder
   std::string clip_projector_type;  // type of projector used in clip model

   /* Adapter Options */
   std::string adapter_type;  // type of adapter used
   float adapter_lora_alpha;  // Alpha value for LoRA adapter 
   uint32_t adapter_control_vector_layer_count;  // Layers in control vector (only for control_vector architecture)
};
}