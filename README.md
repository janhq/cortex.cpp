# Cortex
<p align="center">
  <img alt="cortex-cpplogo" src="https://raw.githubusercontent.com/janhq/cortex/dev/assets/cortex-banner.png">
</p>

<p align="center">
  <a href="https://cortex.so/docs/">Documentation</a> - <a href="https://cortex.so/api-reference">API Reference</a> 
  - <a href="https://github.com/janhq/cortex/releases">Changelog</a> - <a href="https://github.com/janhq/cortex/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> ⚠️ **Cortex is currently in Development**: Expect breaking changes and bugs!

## About
Cortex is a C++ AI engine that comes with a Docker-like command-line interface and client libraries. It supports running AI models using `ONNX`, `TensorRT-LLM`, and `llama.cpp` engines. Cortex can function as a standalone server or be integrated as a library.

## Cortex Engines
Cortex supports the following engines:
- [`cortex.llamacpp`](https://github.com/janhq/cortex.llamacpp): `cortex.llamacpp` library is a C++ inference tool that can be dynamically loaded by any server at runtime. We use this engine to support GGUF inference with GGUF models. The `llama.cpp` is optimized for performance on both CPU and GPU.
- [`cortex.onnx` Repository](https://github.com/janhq/cortex.onnx): `cortex.onnx` is a C++ inference library for Windows that leverages `onnxruntime-genai` and uses DirectML to provide GPU acceleration across a wide range of hardware and drivers, including AMD, Intel, NVIDIA, and Qualcomm GPUs.
- [`cortex.tensorrt-llm`](https://github.com/janhq/cortex.tensorrt-llm): `cortex.tensorrt-llm` is a C++ inference library designed for NVIDIA GPUs. It incorporates NVIDIA’s TensorRT-LLM for GPU-accelerated inference.

## Installation
### MacOs
```bash
brew install cortex-engine
```
### Windows
```bash
winget install cortex-engine
```
### Linux
```bash
sudo apt install cortex-engine
```
### Docker
**Coming Soon!**

### Libraries
- [cortex.js](https://github.com/janhq/cortex.js)
- [cortex.py](https://github.com/janhq/cortex-python)

### Build from Source

To install Cortex from the source, follow the steps below:

1. Clone the Cortex repository [here](https://github.com/janhq/cortex/tree/dev).
2. Navigate to the `cortex-js` folder.
3. Open the terminal and run the following command to build the Cortex project:

```bash
npx nest build
```

4. Make the `command.js` executable:

```bash
chmod +x '[path-to]/cortex/cortex-js/dist/src/command.js'
```

5. Link the package globally:

```bash
npm link
```


## Quickstart
To run and chat with a model in Cortex:
```bash
# Start the Cortex server
cortex

# Start a model
cortex run [model_id]

# Chat with a model
cortex chat [model_id]
```
## Model Library
Cortex supports a list of models available on [Cortex Hub](https://huggingface.co/cortexso).

Here are example of models that you can use based on each supported engine:
### `llama.cpp`
| Model ID         | Variant (Branch) | Model size        | CLI command                        |
|------------------|------------------|-------------------|------------------------------------|
| codestral        | 22b-gguf         | 22B               | `cortex run codestral:22b-gguf`    |
| command-r        | 35b-gguf         | 35B               | `cortex run command-r:35b-gguf`    |
| gemma            | 7b-gguf          | 7B                | `cortex run gemma:7b-gguf`         |
| llama3           | gguf             | 8B                | `cortex run llama3:gguf`           |
| llama3.1         | gguf             | 8B                | `cortex run llama3.1:gguf`         |
| mistral          | 7b-gguf          | 7B                | `cortex run mistral:7b-gguf`       |
| mixtral          | 7x8b-gguf        | 46.7B             | `cortex run mixtral:7x8b-gguf`     |
| openhermes-2.5   | 7b-gguf          | 7B                | `cortex run openhermes-2.5:7b-gguf`|
| phi3             | medium-gguf      | 14B - 4k ctx len  | `cortex run phi3:medium-gguf`      |
| phi3             | mini-gguf        | 3.82B - 4k ctx len| `cortex run phi3:mini-gguf`        |
| qwen2            | 7b-gguf          | 7B                | `cortex run qwen2:7b-gguf`         |
| tinyllama        | 1b-gguf          | 1.1B              | `cortex run tinyllama:1b-gguf`     |
### `ONNX`
| Model ID         | Variant (Branch) | Model size        | CLI command                        |
|------------------|------------------|-------------------|------------------------------------|
| gemma            | 7b-onnx          | 7B                | `cortex run gemma:7b-onnx`         |
| llama3           | onnx             | 8B                | `cortex run llama3:onnx`           |
| mistral          | 7b-onnx          | 7B                | `cortex run mistral:7b-onnx`       |
| openhermes-2.5   | 7b-onnx          | 7B                | `cortex run openhermes-2.5:7b-onnx`|
| phi3             | mini-onnx        | 3.82B - 4k ctx len| `cortex run phi3:mini-onnx`        |
| phi3             | medium-onnx      | 14B - 4k ctx len  | `cortex run phi3:medium-onnx`      |
### `TensorRT-LLM`
| Model ID         | Variant (Branch)              | Model size        | CLI command                        |
|------------------|-------------------------------|-------------------|------------------------------------|
| llama3           | 8b-tensorrt-llm-windows-ampere       | 8B                | `cortex run llama3:8b-tensorrt-llm-windows-ampere`   |
| llama3           | 8b-tensorrt-llm-linux-ampere     | 8B                | `cortex run llama3:8b-tensorrt-llm-linux-ampere` |
| llama3           | 8b-tensorrt-llm-linux-ada   | 8B                | `cortex run llama3:8b-tensorrt-llm-linux-ada`|
| llama3           | 8b-tensorrt-llm-windows-ada       | 8B                | `cortex run llama3:8b-tensorrt-llm-windows-ada`   |
| mistral          | 7b-tensorrt-llm-linux-ampere     | 7B                | `cortex run mistral:7b-tensorrt-llm-linux-ampere`|
| mistral          | 7b-tensorrt-llm-windows-ampere       | 7B                | `cortex run mistral:7b-tensorrt-llm-windows-ampere`  |
| mistral          | 7b-tensorrt-llm-linux-ada   | 7B                | `cortex run mistral:7b-tensorrt-llm-linux-ada`|
| mistral          | 7b-tensorrt-llm-windows-ada       | 7B                | `cortex run mistral:7b-tensorrt-llm-windows-ada`  |
| openhermes-2.5   | 7b-tensorrt-llm-windows-ampere       | 7B                | `cortex run openhermes-2.5:7b-tensorrt-llm-windows-ampere`|
| openhermes-2.5   | 7b-tensorrt-llm-windows-ada     | 7B                | `cortex run openhermes-2.5:7b-tensorrt-llm-windows-ada`|
| openhermes-2.5   | 7b-tensorrt-llm-linux-ada   | 7B                | `cortex run openhermes-2.5:7b-tensorrt-llm-linux-ada`|

> **Note**:
> You should have at least 8 GB of RAM available to run the 7B models, 16 GB to run the 14B models, and 32 GB to run the 32B models.

## Cortex CLI Commands
> **Note**:
> For a more detailed CLI Reference documentation, please see [here](https://cortex.so/docs/cli).
### Start Cortex Server
```bash
cortex 
```
### Chat with a Model
```bash
cortex chat [options] [model_id] [message]
```
### Embeddings
```bash
cortex embeddings [options] [model_id] [message]
```
### Pull a Model
```bash
cortex pull <model_id>
```
> This command can also pulls Hugging Face's models.
### Download and Start a Model
```bash
cortex run [options] [model_id]:[engine]
```
### Get a Model Details
```bash
cortex models get <model_id>
```
### List Models
```bash
cortex models list [options]
```
### Remove a Model
```bash
cortex models remove <model_id>
```
### Start a Model
```bash
cortex models start [model_id]
```
### Stop a Model
```bash
cortex models stop <model_id>
```
### Update a Model Config
```bash
cortex models update [options] <model_id>
```
### Get an Engine Details
```bash
cortex engines <name> get
```
### Install an Engine
```bash
cortex engines <name> install [options]
```
### List Engines
```bash
cortex engines list [options]
```
### Set an Engine Config
```bash
cortex engines <name> set <config> <value>
```
### Show Model Information
```bash
cortex ps
```
### Benchmark the System
```bash
cortex benchmark [options] [model_id]
```
## REST API
Cortex has a REST API that runs at `localhost:1337`.

### Pull a Model
```bash
curl --request POST \
  --url http://localhost:1337/v1/models/{model_id}/pull
```

### Start a Model
```bash
curl --request POST \
  --url http://localhost:1337/v1/models/{model_id}/start \
  --header 'Content-Type: application/json' \
  --data '{
  "prompt_template": "system\n{system_message}\nuser\n{prompt}\nassistant",
  "stop": [],
  "ngl": 4096,
  "ctx_len": 4096,
  "cpu_threads": 10,
  "n_batch": 2048,
  "caching_enabled": true,
  "grp_attn_n": 1,
  "grp_attn_w": 512,
  "mlock": false,
  "flash_attn": true,
  "cache_type": "f16",
  "use_mmap": true,
  "engine": "cortex.llamacpp"
}'
```

### Chat with a Model
```bash
curl http://localhost:1337/v1/chat/completions \
-H "Content-Type: application/json" \
-d '{
  "model": "",
  "messages": [
    {
      "role": "user",
      "content": "Hello"
    },
  ],
  "model": "mistral",
  "stream": true,
  "max_tokens": 1,
  "stop": [
      null
  ],
  "frequency_penalty": 1,
  "presence_penalty": 1,
  "temperature": 1,
  "top_p": 1
}'
```

### Stop a Model
```bash
curl --request POST \
  --url http://localhost:1337/v1/models/mistral/stop
```


> **Note**: Check our [API documentation](https://cortex.so/api-reference) for a full list of available endpoints.

## Contact Support
- For support, please file a [GitHub ticket](https://github.com/janhq/cortex/issues/new/choose).
- For questions, join our Discord [here](https://discord.gg/FTk2MvZwJH).
- For long-form inquiries, please email [hello@jan.ai](mailto:hello@jan.ai).


