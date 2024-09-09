# CortexCPP
<p align="center">
  <img alt="cortex-cpplogo" src="https://raw.githubusercontent.com/janhq/cortex/dev/assets/cortex-banner.png">
</p>

<p align="center">
  <a href="https://cortex.so/docs/">Documentation</a> - <a href="https://cortex.so/api-reference">API Reference</a> 
  - <a href="https://github.com/janhq/cortex.cpp/releases">Changelog</a> - <a href="https://github.com/janhq/cortex.cpp/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> ⚠️ **CortexCPP is currently in Development. This documentation outlines the intended behavior of CortexCPP, which may not yet be fully implemented in the codebase.**

## About
CortexCPP is a Local AI engine that is used to run and customize LLMs. CortexCPP can be deployed as a standalone server, or integrated into apps like [Jan.ai](https://jan.ai/).

CortexCPP supports the following engines:
- [`cortex.llamacpp`](https://github.com/janhq/cortex.llamacpp)
- [`cortex.onnx`](https://github.com/janhq/cortex.onnx)
- [`cortex.tensorrt-llm`](https://github.com/janhq/cortex.tensorrt-llm)

## Installation
To install CortexCPP, download the installer for your operating system from the following options:
- **Stable Version**
  - [Windows]()
  - [Mac]()
  - [Linux (Debian)]()
  - [Linux (Fedora)]()
- **Beta Version**
  - [Windows]()
  - [Mac]()
  - [Linux (Debian)]()
  - [Linux (Fedora)]()
- **Nightly Version**
  - [Windows]()
  - [Mac]()
  - [Linux (Debian)]()
  - [Linux (Fedora)]()


### Libraries
- [cortex.js](https://github.com/janhq/cortex.js)
- [cortex.py](https://github.com/janhq/cortex-python)

### Build from Source

To install CortexCPP from the source, follow the steps below:

1. Clone the CortexCPP repository [here](https://github.com/janhq/cortex.cpp).
2. Navigate to the `engine > vcpkg` folder.
3. Configure the vpkg:

```bash
cd vcpkg
./bootstrap-vcpkg.bat
vcpkg install
```
4. Build the CortexCPP inside the `build` folder:

```bash
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=path_to_vcpkg_folder/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
```
5. Use Visual Studio with the C++ development kit to build the project using the files generated in the `build` folder.

## Quickstart
To run and chat with a model in CortexCPP:
```bash
# Start the CortexCPP server
cortex

# Start a model
cortex run [model_id]
```
## Built-in Model Library
CortexCPP supports a list of models available on [Cortex Hub](https://huggingface.co/cortexso).

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

## CortexCPP CLI Commands

| Command Description                | Command Example                                                     |
|------------------------------------|---------------------------------------------------------------------|
| **Start CortexCPP Server**            | `cortex`                                                            |
| **Chat with a Model**              | `cortex chat [options] [model_id] [message]`                        |
| **Embeddings**                     | `cortex embeddings [options] [model_id] [message]`                  |
| **Pull a Model**                   | `cortex pull <model_id>`                                            |
| **Download and Start a Model**     | `cortex run [options] [model_id]:[engine]`                          |
| **Get Model Details**              | `cortex models get <model_id>`                                      |
| **List Models**                    | `cortex models list [options]`                                      |
| **Delete a Model**                 | `cortex models delete <model_id>`                                   |
| **Start a Model**                  | `cortex models start [model_id]`                                    |
| **Stop a Model**                   | `cortex models stop <model_id>`                                     |
| **Update a Model**            | `cortex models update [options] <model_id>`                         |
| **Get Engine Details**             | `cortex engines get <engine_name>`                                  |
| **Install an Engine**              | `cortex engines install <engine_name> [options]`                    |
| **List Engines**                   | `cortex engines list [options]`                                     |
| **Uninnstall an Engine**              | `cortex engines uninstall <engine_name> [options]`                 |
| **Show Model Information**         | `cortex ps`                                                         |
| **Update CortexCPP**         | `cortex update [options]`                                                         |

> **Note**:
> For a more detailed CLI Reference documentation, please see [here](https://cortex.so/docs/cli).

## REST API
CortexCPP has a REST API that runs at `localhost:3928`.

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


