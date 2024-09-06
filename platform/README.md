# Cortex
<!-- <p align="center">
  <img alt="cortex-cpplogo" src="https://raw.githubusercontent.com/janhq/cortex/dev/assets/cortex-banner.png">
</p> -->

<p align="center">
  <a href="https://cortex.so/docs/cortex-platform/">Documentation</a> - <a href="https://cortex.so/api-reference">API Reference</a> 
  - <a href="https://github.com/janhq/cortex.cpp/releases">Changelog</a> - <a href="https://github.com/janhq/cortex.cpp/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> ⚠️ **Cortex Platform is under development**

## About
Cortex Platform is a fully database-driven application built on top of [`cortex.cpp`](https://github.com/janhq/cortex.cpp), designed as an OpenAI API equivalent. It supports multiple engines, multi-user functionality, and operates entirely through stateful API endpoints.

## Cortex Engines
Cortex Platform supports the following engines:
- [`cortex.llamacpp`](https://github.com/janhq/cortex.llamacpp): `cortex.llamacpp` library is a C++ inference tool that can be dynamically loaded by any server at runtime. We use this engine to support GGUF inference with GGUF models. The `llama.cpp` is optimized for performance on both CPU and GPU.
- [`cortex.onnx` Repository](https://github.com/janhq/cortex.onnx): `cortex.onnx` is a C++ inference library for Windows that leverages `onnxruntime-genai` and uses DirectML to provide GPU acceleration across a wide range of hardware and drivers, including AMD, Intel, NVIDIA, and Qualcomm GPUs.
- [`cortex.tensorrt-llm`](https://github.com/janhq/cortex.tensorrt-llm): `cortex.tensorrt-llm` is a C++ inference library designed for NVIDIA GPUs. It incorporates NVIDIA’s TensorRT-LLM for GPU-accelerated inference.

## Installation

> **Note**:
> To install the Cortex Platform, clone our [repository](). It includes everything you need for installation using Docker and Helm.

### Docker
```bash
docker compose up
```

### Helm
```bash
helm install cortex-platform
```

### Yarn
```bash
yarn install cortex-platform
```

### Libraries
- [cortex.js]()
- [cortex.py]()


### Build from Source
**Coming Soon!**

## Quickstart
**Coming Soon!**

## Model Library
Cortex Platform supports a list of models available on [Cortex Hub](https://huggingface.co/cortexso).

Here are example of models that you can use based on each supported engine:
### `llama.cpp`
| Model ID         | Variant (Branch) | Model size        |
|------------------|------------------|-------------------|
| codestral        | 22b-gguf         | 22B               |
| command-r        | 35b-gguf         | 35B               |
| gemma            | 7b-gguf          | 7B                |
| llama3           | gguf             | 8B                |
| llama3.1         | gguf             | 8B                |
| mistral          | 7b-gguf          | 7B                |
| mixtral          | 7x8b-gguf        | 46.7B             |
| openhermes-2.5   | 7b-gguf          | 7B                |
| phi3             | medium-gguf      | 14B - 4k ctx len  |
| phi3             | mini-gguf        | 3.82B - 4k ctx len|
| qwen2            | 7b-gguf          | 7B                |
| tinyllama        | 1b-gguf          | 1.1B              |

### `ONNX`
| Model ID         | Variant (Branch) | Model size        |
|------------------|------------------|-------------------|
| gemma            | 7b-onnx          | 7B                |
| llama3           | onnx             | 8B                |
| mistral          | 7b-onnx          | 7B                |
| openhermes-2.5   | 7b-onnx          | 7B                |
| phi3             | mini-onnx        | 3.82B - 4k ctx len|
| phi3             | medium-onnx      | 14B - 4k ctx len  |

### `TensorRT-LLM`
| Model ID         | Variant (Branch)              | Model size        |
|------------------|-------------------------------|-------------------|
| llama3           | 8b-tensorrt-llm-windows-ampere | 8B                |
| llama3           | 8b-tensorrt-llm-linux-ampere   | 8B                |
| llama3           | 8b-tensorrt-llm-linux-ada      | 8B                |
| llama3           | 8b-tensorrt-llm-windows-ada    | 8B                |
| mistral          | 7b-tensorrt-llm-linux-ampere   | 7B                |
| mistral          | 7b-tensorrt-llm-windows-ampere | 7B                |
| mistral          | 7b-tensorrt-llm-linux-ada      | 7B                |
| mistral          | 7b-tensorrt-llm-windows-ada    | 7B                |
| openhermes-2.5   | 7b-tensorrt-llm-windows-ampere | 7B                |
| openhermes-2.5   | 7b-tensorrt-llm-windows-ada    | 7B                |
| openhermes-2.5   | 7b-tensorrt-llm-linux-ada      | 7B                |

> **Note**:
> You should have at least 8 GB of RAM available to run the 7B models, 16 GB to run the 14B models, and 32 GB to run the 32B models.

## Cortex Platfrom API
Cortex Platform only support the following stateful API endpoints:

- Messages
- Threads
- Assistants

Here are some examples of the available stateful endpoints:
### Create Message
```bash
curl --request POST \
  --url http://127.0.0.1:1337/v1/threads/__THREAD_ID__/messages \
  --header 'Content-Type: application/json' \
  --data '{
  "role": "user",
  "content": "Tell me a joke"
}'
```

### Create Assistant
```bash
curl --request POST \
  --url http://127.0.0.1:1337/v1/assistants \
  --header 'Content-Type: application/json' \
  --data '{
  "id": "jan",
  "avatar": "",
  "name": "Jan",
  "description": "A default assistant that can use all downloaded models",
  "model": "",
  "instructions": "",
  "tools": [],
  "metadata": {},
  "top_p": "0.7",
  "temperature": "0.7"
}'
```

### Create Thread
```bash
curl --request POST \
  --url http://127.0.0.1:1337/v1/threads \
  --header 'Content-Type: application/json' \
  --data '{
  "assistants": [
    {
      "id": "thread_123",
      "avatar": "https://example.com/avatar.png",
      "name": "Virtual Helper",
      "model": "mistral",
      "instructions": "Assist with customer queries and provide information based on the company database.",
      "tools": [
        {
          "name": "Knowledge Retrieval",
          "settings": {
            "source": "internal",
            "endpoint": "https://api.example.com/knowledge"
          }
        }
      ],
      "description": "This assistant helps with customer support by retrieving relevant information.",
      "metadata": {
        "department": "support",
        "version": "1.0"
      },
      "object": "assistant",
      "temperature": 0.7,
      "top_p": 0.9,
      "created_at": 1622470423,
      "response_format": {
        "format": "json"
      },
      "tool_resources": {
        "resources": [
          "database1",
          "database2"
        ]
      }
    }
  ]
}'
```

> **Note**: Check our [API documentation](https://cortex.so/api-reference) for a full list of available stateful endpoints.

## Contact Support
- For support, please file a [GitHub ticket](https://github.com/janhq/cortex/issues/new/choose).
- For questions, join our Discord [here](https://discord.gg/FTk2MvZwJH).
- For long-form inquiries, please email [hello@jan.ai](mailto:hello@jan.ai).


