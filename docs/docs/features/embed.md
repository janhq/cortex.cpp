---
title: Embedding
---

## Activating Embedding Feature

To activate the embedding feature in Nitro, a JSON parameter `"embedding": true` needs to be included in the inference request. This setting allows Nitro to process inferences with embedding enabled, enhancing the model's capabilities.

### Example Request

Here’s an example showing how to enable the embedding feature:

```zsh
curl -X POST 'http://localhost:3928/inferences/llamacpp/loadmodel' \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
    "embedding": true,
    "pre_prompt": "A chat between a curious user and an artificial intelligence",
    "user_prompt": "USER: "
  }'
```

The response will be
```

```
