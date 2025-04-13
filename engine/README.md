# cortex-cpp - Embeddable AI
<p align="center">
  <img alt="cortex-cpplogo" src="https://raw.githubusercontent.com/menloresearch/cortex.cpp/dev/assets/cortex-banner.png">
</p>

<p align="center">
  <a href="https://jan.ai/cortex">Documentation</a> - <a href="https://jan.ai/api-reference">API Reference</a> 
  - <a href="https://github.com/menloresearch/cortex.cpp/releases">Changelog</a> - <a href="https://github.com/menloresearch/cortex.cpp/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> ⚠️ **cortex-cpp is currently in Development**: Expect breaking changes and bugs!

## About cortex-cpp

Cortex-cpp is a streamlined, stateless C++ server engineered to be fully compatible with OpenAI's API, particularly its stateless functionalities. It integrates a Drogon server framework to manage request handling and includes features like model orchestration and hardware telemetry, which are essential for production environments.

Remarkably compact, the binary size of cortex-cpp is around 3 MB when compressed, with minimal dependencies. This lightweight and efficient design makes cortex-cpp an excellent choice for deployments in both edge computing and server contexts.

> Utilizing GPU capabilities does require CUDA.

## Prerequisites
### **Hardware**

Ensure that your system meets the following requirements to run Cortex:

- **OS**:
  - MacOSX 13.6 or higher.
  - Windows 10 or higher.
  - Ubuntu 18.04 and later.
- **RAM (CPU Mode):**
  - 8GB for running up to 3B models.
  - 16GB for running up to 7B models.
  - 32GB for running up to 13B models.
- **VRAM (GPU Mode):**

  - 6GB can load the 3B model (int4) with `ngl` at 120 ~ full speed on CPU/ GPU.
  - 8GB can load the 7B model (int4) with `ngl` at 120 ~ full speed on CPU/ GPU.
  - 12GB can load the 13B model (int4) with `ngl` at 120 ~ full speed on CPU/ GPU.

- **Disk**: At least 10GB for app and model download.

## Quickstart
To install Cortex CLI, follow the steps below:
1. Download cortex-cpp here: https://github.com/menloresearch/cortex.cpp/releases
2. Install cortex-cpp by running the downloaded file.

3. Download a Model:

```bash
mkdir model && cd model
wget -O llama-2-7b-model.gguf https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/resolve/main/llama-2-7b-chat.Q5_K_M.gguf?download=true
```

4. Run cortex-cpp server:

```bash title="Run cortex-cpp server"
cortex-cpp
```

5. Load a model:

```bash title="Load model"
curl http://localhost:3928/inferences/server/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/model/llama-2-7b-model.gguf",
    "ctx_len": 512,
    "ngl": 100,
  }'
```

6. Make an Inference:

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

## Table of parameters
Below is the available list of the model parameters you can set when loading a model in cortex-cpp:

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
      <a href='https://github.com/menloresearch/cortex.cpp/releases/download/v0.4.12/cortex-cpp-0.4.12-windows-amd64-avx2.tar.gz'>
        <img src='./docs/static/img/windows.png' style="height:15px; width: 15px" />
        <b>CPU</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/menloresearch/cortex.cpp/releases/download/v0.4.12/cortex-cpp-0.4.12-windows-amd64-avx2-cuda-12-0.tar.gz'>
        <img src='./docs/static/img/windows.png' style="height:15px; width: 15px" />
        <b>CUDA</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/menloresearch/cortex.cpp/releases/download/v0.4.12/cortex-cpp-0.4.12-mac-amd64.tar.gz'>
        <img src='./docs/static/img/mac.png' style="height:15px; width: 15px" />
        <b>Intel</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/menloresearch/cortex.cpp/releases/download/v0.4.12/cortex-cpp-0.4.12-mac-arm64.tar.gz'>
        <img src='./docs/static/img/mac.png' style="height:15px; width: 15px" />
        <b>M1/M2</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/menloresearch/cortex.cpp/releases/download/v0.4.12/cortex-cpp-0.4.12-linux-amd64-avx2.tar.gz'>
        <img src='./docs/static/img/linux.png' style="height:15px; width: 15px" />
        <b>CPU</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/menloresearch/cortex.cpp/releases/download/v0.4.12/cortex-cpp-0.4.12-linux-amd64-cuda-12-0.tar.gz'>
        <img src='./docs/static/img/linux.png' style="height:15px; width: 15px" />
        <b>CUDA</b>
      </a>
    </td>
  </tr>
</table>

> Download the latest or older versions of Cortex-cpp at the **[GitHub Releases](https://github.com/menloresearch/cortex.cpp/releases)**.


## Manual Build
Manual build is a process in which the developers build the software manually. This is usually done when a new feature is implemented, or a bug is fixed. The process for this project is defined in [`.github/workflows/cortex-build.yml`](../.github/workflows/cortex-build.yml)

## Contact Support

- For support, please file a GitHub ticket.
- For questions, join our Discord [here](https://discord.gg/FTk2MvZwJH).
- For long-form inquiries, please email hello@jan.ai.

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=menloresearch/cortex.cpp&type=Date)](https://star-history.com/#menloresearch/cortex.cpp&Date)