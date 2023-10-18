# Nitro - Accelerated AI Inference Engine

<p align="center">
  <img alt="nitrologo" src="https://user-images.githubusercontent.com/69952136/266939567-4a7d24f0-9338-4ab5-9261-cb3c71effe35.png">
</p>

<p align="center">
  <a href="https://docs.jan.ai/">Getting Started</a> - <a href="https://docs.jan.ai">Docs</a> 
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
    "embedding": true
  }'
```

`ctx_len` and `ngl` are typical llama C++ parameters, and `embedding` determines whether to enable the embedding endpoint or not.

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

## About Nitro

### Repo Structure

```
.
├── controllers
├── docs 
├── llama.cpp -> Upstream llama C++
├── nitro_deps -> Dependencies of the Nitro project as a sub-project
└── utils
```

### Architecture
Nitro is an integration layer with the most cutting-edge inference engine. Its structure can be simplified as follows:

![Current architecture](docs/architecture.png)

### Contact

- For support, please file a GitHub ticket.
- For questions, join our Discord [here](https://discord.gg/FTk2MvZwJH).
- For long-form inquiries, please email hello@jan.ai.

