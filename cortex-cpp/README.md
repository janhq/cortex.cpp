# cortex-cpp - Embeddable AI
<p align="center">
  <img alt="cortex-cpplogo" src="https://raw.githubusercontent.com/janhq/cortex-cpp/main/assets/Cortex-cpp%20README%20banner.png">
</p>

<p align="center">
  <a href="https://jan.ai/cortex">Documentation</a> - <a href="https://jan.ai/api-reference">API Reference</a> 
  - <a href="https://github.com/janhq/cortex/releases">Changelog</a> - <a href="https://github.com/janhq/cortex/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> ⚠️ **cortex-cpp is currently in Development**: Expect breaking changes and bugs!

## About cortex-cpp

Cortex-cpp is a streamlined, stateless C++ server engineered to be fully compatible with OpenAI's API, particularly its stateless functionalities. It integrates a Drogon server framework to manage request handling and includes features like model orchestration and hardware telemetry, which are essential for production environments.

Remarkably compact, the binary size of cortex-cpp is around 3 MB when compressed, with minimal dependencies. This lightweight and efficient design makes cortex-cpp an excellent choice for deployments in both edge computing and server contexts.

> Utilizing GPU capabilities does require CUDA.

### Features
- Fast Inference: Built on top of the cutting-edge inference library llama.cpp, modified to be production-ready.
- Lightweight: Only 3MB, ideal for resource-sensitive environments.
- Easily Embeddable: Simple integration into existing applications, offering flexibility.
- Quick Setup: Approximately 10-second initialization for swift deployment.
- Enhanced Web Framework: Incorporates drogon cpp to boost web service efficiency.

### Repo Structure

```
.
├── common          # Common libraries or shared resources
├── controllers     # Controller scripts or modules for managing interactions
├── cortex-common   # Shared components across different cortex modules
├── cortex-cpp-deps # Dependencies specific to the cortex-cpp module
├── engines         # Different processing or computational engines
├── examples        # Example scripts or applications demonstrating usage
├── test            # Test scripts and testing frameworks
└── utils           # Utility scripts and helper functions

```

## Quickstart

**Step 1: Install cortex-cpp**

Download cortex-cpp here: https://github.com/janhq/cortex/releases

**Step 2: Downloading a Model**

```bash
mkdir model && cd model
wget -O llama-2-7b-model.gguf https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/resolve/main/llama-2-7b-chat.Q5_K_M.gguf?download=true
```

**Step 3: Run cortex-cpp server**

```bash title="Run cortex-cpp server"
cortex-cpp
```

**Step 4: Load model** 

```bash title="Load model"
curl http://localhost:3928/inferences/server/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/model/llama-2-7b-model.gguf",
    "ctx_len": 512,
    "ngl": 100,
  }'
```

**Step 5: Making an Inference**

```bash title="cortex-cpp Inference"
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

Table of parameters

| Parameter        | Type    | Description                                                  |
|------------------|---------|--------------------------------------------------------------|
| `llama_model_path` | String  | The file path to the LLaMA model.                            |
| `ngl`              | Integer | The number of GPU layers to use.                             |
| `ctx_len`          | Integer | The context length for the model operations.                 |
| `embedding`        | Boolean | Whether to use embedding in the model.                       |
| `n_parallel`       | Integer | The number of parallel operations. |
| `cont_batching`    | Boolean | Whether to use continuous batching.                          |
| `user_prompt`      | String  | The prompt to use for the user.                              |
| `ai_prompt`        | String  | The prompt to use for the AI assistant.                      |
| `system_prompt`    | String  | The prompt to use for system rules.                          |
| `pre_prompt`    | String  | The prompt to use for internal configuration.                          |
| `cpu_threads`   | Integer | The number of threads to use for inferencing (CPU MODE ONLY) |
| `n_batch`       | Integer | The batch size for prompt eval step |
| `caching_enabled` | Boolean | To enable prompt caching or not   |
| `clean_cache_threshold` | Integer | Number of chats that will trigger clean cache action|
|`grp_attn_n`|Integer|Group attention factor in self-extend|
|`grp_attn_w`|Integer|Group attention width in self-extend|
|`mlock`|Boolean|Prevent system swapping of the model to disk in macOS|
|`grammar_file`| String |You can constrain the sampling using GBNF grammars by providing path to a grammar file|
|`model_type` | String | Model type we want to use: llm or embedding, default value is llm|

***OPTIONAL***: You can run Cortex-cpp on a different port like 5000 instead of 3928 by running it manually in terminal
```zsh
./cortex-cpp 1 127.0.0.1 5000 ([thread_num] [host] [port] [uploads_folder_path])
```
- thread_num : the number of thread that cortex-cpp webserver needs to have
- host : host value normally 127.0.0.1 or 0.0.0.0
- port : the port that cortex-cpp got deployed onto
- uploads_folder_path: custom path for file uploads in Drogon.

cortex-cpp server is compatible with the OpenAI format, so you can expect the same output as the OpenAI ChatGPT API.

## Download

<table>
  <tr>
    <td style="text-align:center"><b>Version Type</b></td>
    <td colspan="2" style="text-align:center"><b>Windows</b></td>
    <td colspan="2" style="text-align:center"><b>MacOS</b></td>
    <td colspan="2" style="text-align:center"><b>Linux</b></td>
  </tr>
  <tr>
    <td style="text-align:center"><b>Stable (Recommended)</b></td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/cortex/releases/download/v0.4.12/cortex-cpp-0.4.12-windows-amd64-avx2.tar.gz'>
        <img src='./docs/static/img/windows.png' style="height:15px; width: 15px" />
        <b>CPU</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/cortex/releases/download/v0.4.12/cortex-cpp-0.4.12-windows-amd64-avx2-cuda-12-0.tar.gz'>
        <img src='./docs/static/img/windows.png' style="height:15px; width: 15px" />
        <b>CUDA</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/cortex/releases/download/v0.4.12/cortex-cpp-0.4.12-mac-amd64.tar.gz'>
        <img src='./docs/static/img/mac.png' style="height:15px; width: 15px" />
        <b>Intel</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/cortex/releases/download/v0.4.12/cortex-cpp-0.4.12-mac-arm64.tar.gz'>
        <img src='./docs/static/img/mac.png' style="height:15px; width: 15px" />
        <b>M1/M2</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/cortex/releases/download/v0.4.12/cortex-cpp-0.4.12-linux-amd64-avx2.tar.gz'>
        <img src='./docs/static/img/linux.png' style="height:15px; width: 15px" />
        <b>CPU</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/cortex/releases/download/v0.4.12/cortex-cpp-0.4.12-linux-amd64-cuda-12-0.tar.gz'>
        <img src='./docs/static/img/linux.png' style="height:15px; width: 15px" />
        <b>CUDA</b>
      </a>
    </td>
  </tr>
</table>

Download the latest version of Cortex-cpp at https://jan.ai/ or visit the **[GitHub Releases](https://github.com/janhq/cortex/releases)** to download any previous release.


## Manual Build
Manual build is a process in which the developers build the software manually. This is usually done when a new feature is implemented, or a bug is fixed. The process for this project is defined in [`.github/workflows/build.yml`](.github/workflows/build.yml)

## Contact Support

- For support, please file a GitHub ticket.
- For questions, join our Discord [here](https://discord.gg/FTk2MvZwJH).
- For long-form inquiries, please email hello@jan.ai.

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=janhq/cortex-cpp&type=Date)](https://star-history.com/#janhq/cortex-cpp&Date)
