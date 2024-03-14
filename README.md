# Nitro - Embeddable AI
<p align="center">
  <img alt="nitrologo" src="https://raw.githubusercontent.com/janhq/nitro/main/assets/Nitro%20README%20banner.png">
</p>

<p align="center">
  <a href="https://nitro.jan.ai/docs">Documentation</a> - <a href="https://nitro.jan.ai/api-reference">API Reference</a> 
  - <a href="https://github.com/janhq/nitro/releases/">Changelog</a> - <a href="https://github.com/janhq/nitro/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> ⚠️ **Nitro is currently in Development**: Expect breaking changes and bugs!

## Features
- Fast Inference: Built on top of the cutting-edge inference library llama.cpp, modified to be production ready.
- Lightweight: Only 3MB, ideal for resource-sensitive environments.
- Easily Embeddable: Simple integration into existing applications, offering flexibility.
- Quick Setup: Approximately 10-second initialization for swift deployment.
- Enhanced Web Framework: Incorporates drogon cpp to boost web service efficiency.

## About Nitro

Nitro is a high-efficiency C++ inference engine for edge computing, powering [Jan](https://jan.ai/). It is lightweight and embeddable, ideal for product integration.

The binary of nitro after zipped is only ~3mb in size with none to minimal dependencies (if you use a GPU need CUDA for example) make it desirable for any edge/server deployment 👍.

> Read more about Nitro at https://nitro.jan.ai/

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

**Step 1: Install Nitro**

- For Linux and MacOS

  ```bash
  curl -sfL https://raw.githubusercontent.com/janhq/nitro/main/install.sh | sudo /bin/bash -
  ```

- For Windows

  ```bash
  powershell -Command "& { Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/janhq/nitro/main/install.bat' -OutFile 'install.bat'; .\install.bat; Remove-Item -Path 'install.bat' }"
  ```

**Step 2: Downloading a Model**

```bash
mkdir model && cd model
wget -O llama-2-7b-model.gguf https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/resolve/main/llama-2-7b-chat.Q5_K_M.gguf?download=true
```

**Step 3: Run Nitro server**

```bash title="Run Nitro server"
nitro
```

**Step 4: Load model** 

```bash title="Load model"
curl http://localhost:3928/inferences/llamacpp/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/model/llama-2-7b-model.gguf",
    "ctx_len": 512,
    "ngl": 100,
  }'
```

**Step 5: Making an Inference**

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

***OPTIONAL***: You can run Nitro on a different port like 5000 instead of 3928 by running it manually in terminal
```zsh
./nitro 1 127.0.0.1 5000 ([thread_num] [host] [port] [uploads_folder_path])
```
- thread_num : the number of thread that nitro webserver needs to have
- host : host value normally 127.0.0.1 or 0.0.0.0
- port : the port that nitro got deployed onto
- uploads_folder_path: custom path for file uploads in Drogon.

Nitro server is compatible with the OpenAI format, so you can expect the same output as the OpenAI ChatGPT API.

## Compile from source
To compile nitro please visit [Compile from source](docs/docs/new/build-source.md)

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
      <a href='https://github.com/janhq/nitro/releases/download/v0.3.14/nitro-0.3.14-win-amd64.tar.gz'>
        <img src='./docs/static/img/windows.png' style="height:15px; width: 15px" />
        <b>CPU</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/nitro/releases/download/v0.3.14/nitro-0.3.14-win-amd64-cuda.tar.gz'>
        <img src='./docs/static/img/windows.png' style="height:15px; width: 15px" />
        <b>CUDA</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/nitro/releases/download/v0.3.14/nitro-0.3.14-mac-amd64.tar.gz'>
        <img src='./docs/static/img/mac.png' style="height:15px; width: 15px" />
        <b>Intel</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/nitro/releases/download/v0.3.14/nitro-0.3.14-mac-arm64.tar.gz'>
        <img src='./docs/static/img/mac.png' style="height:15px; width: 15px" />
        <b>M1/M2</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/nitro/releases/download/v0.3.14/nitro-0.3.14-linux-amd64.tar.gz'>
        <img src='./docs/static/img/linux.png' style="height:15px; width: 15px" />
        <b>CPU</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://github.com/janhq/nitro/releases/download/v0.3.14/nitro-0.3.14-linux-amd64-cuda.tar.gz'>
        <img src='./docs/static/img/linux.png' style="height:15px; width: 15px" />
        <b>CUDA</b>
      </a>
    </td>
  </tr>
  <tr style="text-align: center">
    <td style="text-align:center"><b>Experimental (Nighlty Build)</b></td>
    <td style="text-align:center" colspan="6">
      <a href='https://github.com/janhq/nitro/actions/runs/8146271749'>
        <b>GitHub action artifactory</b>
      </a>
    </td>
  </tr>
</table>

Download the latest version of Nitro at https://nitro.jan.ai/ or visit the **[GitHub Releases](https://github.com/janhq/nitro/releases)** to download any previous release.

## Nightly Build

Nightly build is a process where the software is built automatically every night. This helps in detecting and fixing bugs early in the development cycle. The process for this project is defined in [`.github/workflows/build.yml`](.github/workflows/build.yml)

You can join our Discord server [here](https://discord.gg/FTk2MvZwJH) and go to channel [github-nitro](https://discordapp.com/channels/1107178041848909847/1151022176019939328) to monitor the build process.

The nightly build is triggered at 2:00 AM UTC every day.

The nightly build can be downloaded from the url notified in the Discord channel. Please access the url from the browser and download the build artifacts from there.

## Manual Build

Manual build is a process where the software is built manually by the developers. This is usually done when a new feature is implemented or a bug is fixed. The process for this project is defined in [`.github/workflows/build.yml`](.github/workflows/build.yml)

It is similar to the nightly build process, except that it is triggered manually by the developers.

### Contact

- For support, please file a GitHub ticket.
- For questions, join our Discord [here](https://discord.gg/FTk2MvZwJH).
- For long-form inquiries, please email hello@jan.ai.

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=janhq/nitro&type=Date)](https://star-history.com/#janhq/nitro&Date)
