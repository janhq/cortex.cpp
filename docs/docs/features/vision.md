---
title: Vision
description: Inference engine for vision, the same as OpenAI's
keywords: [Nitro, Jan, fast inference, inference server, local AI, large language model, OpenAI compatible, open source, llava, bakllava, vision]
---

## Load model
Just like loading the Chat model, for the vision model, you need two specific types:
- the `GGUF model`
- the `mmproj model`.

You can load the model using:

```bash title="Load Model" {3,4}
curl -X POST 'http://127.0.0.1:3928/inferences/llamacpp/loadmodel' -H 'Content-Type: application/json' -d '{
   "llama_model_path": "/path/to/gguf/model/",
   "mmproj": "/path/to/mmproj/model/",
   "ctx_len": 2048,
   "ngl": 100,
   "cont_batching": false,
   "embedding": false,
   "system_prompt": "",
   "user_prompt": "\n### Instruction:\n",
   "ai_prompt": "\n### Response:\n"
 }'
```

Download the models here:
- [Llava Model](https://huggingface.co/jartine/llava-v1.5-7B-GGUF/tree/main): Large Language and Vision Assistant achieves SoTA on 11 benchmarks.
- [Bakllava Model](https://huggingface.co/mys/ggml_bakllava-1/tree/main) is a Mistral 7B base augmented with the LLaVA architecture.

## Inference

Nitro currently only works with images converted to base64 format. Use this [base64 converter](https://www.base64-image.de/) to prepare your images.

To get the model's understanding of an image, do the following:

```bash title="Inference"
curl http://127.0.0.1:3928/v1/chat/completions \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $OPENAI_API_KEY" \
  -d '{
    "model": "gpt-4-vision-preview",
    "messages": [
      {
        "role": "user",
        "content": [
          {
            "type": "text",
            "text": "What’s in this image?"
          },
          {
            "type": "image_url",
            "image_url": {
              "url": "<base64>"
            }
          }
        ]
      }
    ],
    "max_tokens": 300
  }'
```

If the base64 string is too long and causes errors, consider using [Postman](https://www.postman.com/) as an alternative.