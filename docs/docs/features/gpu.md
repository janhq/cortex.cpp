---
title: GPU inference
---

## Enabling GPU Inference

To enable GPU inference in Nitro, a simple POST request is used. This request will instruct Nitro to load the specified model into the GPU, significantly boosting the inference throughput.

```zsh title="GPU enable" {5}
curl -X POST 'http://localhost:3928/inferences/llamacpp/loadmodel' \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
    "ngl": 100
  }'
```

You can adjust the `ngl` parameter based on your requirements and GPU capabilities.

## CPU inference vs GPU inference benchmark

:::info
Updating
:::