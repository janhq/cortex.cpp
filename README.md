# Nitro - Accelerated AI Inference Engine

<p align="center">
  <img alt="nitrologo" src="https://user-images.githubusercontent.com/69952136/266939567-4a7d24f0-9338-4ab5-9261-cb3c71effe35.png">
</p>

<p align="center">
  <a href="https://jan.ai/nitro">Getting Started</a> - <a href="https://jan.ai/nitro">Docs</a> 
  - <a href="https://docs.jan.ai/changelog/">Changelog</a> - <a href="https://github.com/janhq/nitro/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> ⚠️ **Nitro is currently in Development**: Expect breaking changes and bugs!

## Features

### Supported features
- GGML inference support (llama.cpp, etc...)

### TODO:
- [ ] Local file server
- [ ] Cache
- [ ] Plugin support

## Documentation

## About Nitro

Nitro is a light-weight integration layer (and soon to be inference engine) for cutting edge inference engine, make deployment of AI models easier than ever before!

The binary of nitro after zipped is only ~3mb in size with none to minimal dependencies (if you use a GPU need CUDA for example) make it desirable for any edge/server deployment 👍.

### Repo Structure

```
.
├── controllers
├── docs 
├── llama.cpp -> Upstream llama C++
├── nitro_deps -> Dependencies of the Nitro project as a sub-project
└── utils
```

## Quickstart

**Step 1: Download Nitro**

To use Nitro, download the released binaries from the release page below:

[![Download Nitro](https://img.shields.io/badge/Download-Nitro-blue.svg)](https://github.com/janhq/nitro/releases)

After downloading the release, double-click on the Nitro binary.

**Step 2: Download a Model**

Download a llama model to try running the llama C++ integration. You can find a "GGUF" model on The Bloke's page below:

[![Download Model](https://img.shields.io/badge/Download-Model-green.svg)](https://huggingface.co/TheBloke)

**Step 3: Run Nitro**

Double-click on Nitro to run it. After downloading your model, make sure it's saved to a specific path. Then, make an API call to load your model into Nitro.

```zsh
curl -X POST 'http://localhost:3928/inferences/llamacpp/loadmodel' \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
    "ctx_len": 2048,
    "ngl": 100,
    "embedding": true,
    "n_parallel": 4,
    "pre_prompt": "A chat between a curious user and an artificial intelligence",
    "user_prompt": "what is AI?"
  }'
```

Table of parameters

| Parameter        | Type    | Description                                                  |
|------------------|---------|--------------------------------------------------------------|
| `llama_model_path` | String  | The file path to the LLaMA model.                            |
| `ngl`              | Integer | The number of GPU layers to use.                             |
| `ctx_len`          | Integer | The context length for the model operations.                 |
| `embedding`        | Boolean | Whether to use embedding in the model.                       |
| `n_parallel`       | Integer | The number of parallel operations. Uses Drogon thread count if not set. |
| `cont_batching`    | Boolean | Whether to use continuous batching.                          |
| `user_prompt`      | String  | The prompt to use for the user.                              |
| `ai_prompt`        | String  | The prompt to use for the AI assistant.                      |
| `system_prompt`    | String  | The prompt to use for system rules.                          |
| `pre_prompt`    | String  | The prompt to use for internal configuration.                          |

**Step 4: Perform Inference on Nitro for the First Time**

```zsh
curl --location 'http://localhost:3928/inferences/llamacpp/chat_completion' \
     --header 'Content-Type: application/json' \
     --header 'Accept: text/event-stream' \
     --header 'Access-Control-Allow-Origin: *' \
     --data '{
        "messages": [
            {"content": "Hello there 👋", "role": "assistant"},
            {"content": "Can you write a long story", "role": "user"}
        ],
        "stream": true,
        "model": "gpt-3.5-turbo",
        "max_tokens": 2000
     }'
```

Nitro server is compatible with the OpenAI format, so you can expect the same output as the OpenAI ChatGPT API.

## Compile from source
To compile nitro please visit [Compile from source](docs/manual_install.md)

### Architecture
Nitro is an integration layer with the most cutting-edge inference engine. Its structure can be simplified as follows:

![Current architecture](docs/architecture.png)

### Contact

- For support, please file a GitHub ticket.
- For questions, join our Discord [here](https://discord.gg/FTk2MvZwJH).
- For long-form inquiries, please email hello@jan.ai.

