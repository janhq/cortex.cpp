---
title: Self extend
description: Self-Extend LLM Context Window Without Tuning
keywords: [long context, longlm, Nitro, Jan, fast inference, inference server, local AI, large language model, OpenAI compatible, open source, llama]
---

## Enhancing LLMs with Self-Extend
Self-Extend offers an innovative approach to increase the context window of Large Language Models (LLMs) without the usual need for re-tuning. This method adapts the attention mechanism during the inference phase and eliminates the necessity for additional training or fine-tuning.

For in-depth technical insights, refer to their research [paper](https://arxiv.org/pdf/2401.01325.pdf).

## Activating Self-Extend for LLMs

To activate the Self-Extend feature while loading your model, use the following command:

```bash title="Enable Self-Extend" {6,7}
curl http://localhost:3928/inferences/llamacpp/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
    "ctx_len": 8192,
    "grp_attn_n": 4,
    "grp_attn_w": 2048,
  }'
```

**Note:** 
- For optimal performance, `grp_attn_w` should be as large as possible, but smaller than the training context length.
- Setting  `grp_attn_n` between 2 to 4 is recommended for peak efficiency. Higher values may result in increased incoherence in output.