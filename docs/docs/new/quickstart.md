---
title: Quickstart
slug: /quickstart
description: How to use Nitro
keywords: [Nitro, Jan, fast inference, inference server, local AI, large language model, OpenAI compatible, open source, llama]
---

## Step 1: Install Nitro

Download and install Nitro on your system.

### From the release page

You can directly choose the pre-built binary that compatible with your system at

> [Nitro Release Page](https://github.com/janhq/nitro/releases)

After you have downloaded the binary, you can directly use the binary with "./nitro".

If you want to build from source rather than using the pre-built binary, you can also check: [Install from Source](install.md)

## Step 2: Downloading a Model

For this example, we'll use the [Llama2 7B chat model](https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/tree/main).

```bash
mkdir model && cd model
wget -O llama-2-7b-model.gguf https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/resolve/main/llama-2-7b-chat.Q5_K_M.gguf?download=true
```

## Step 3: Run Nitro server

```bash title="Run Nitro server"
nitro
```

To check if the Nitro server is running:

```bash title="Nitro Health Status"
curl http://localhost:3928/healthz
```

## Step 4: Load model

To load the model to Nitro server, run:

```bash title="Load model"
curl http://localhost:3928/inferences/llamacpp/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/model/llama-2-7b-model.gguf",
    "ctx_len": 512,
    "ngl": 100,
  }'
```

## Step 5: Making an Inference

Finally, let's chat with the model using Nitro.

```bash title="Nitro Inference"
curl http://localhost:3928/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "messages": [
      {
        "role": "user",
        "content": "Who won the world series in 2020?"
      },
    ]
  }'
```

As you can see, a key benefit of Nitro is its alignment with [OpenAI's API structure](https://platform.openai.com/docs/guides/text-generation?lang=curl). Its inference call syntax closely mirrors that of OpenAI's API, facilitating an easier shift for those accustomed to OpenAI's framework.
