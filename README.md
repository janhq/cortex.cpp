# Cortex - CLI

<p align="center">
  <a href="https://jan.ai/cortex">Documentation</a> - <a href="https://jan.ai/api-reference">API Reference</a> 
  - <a href="https://github.com/janhq/cortex/releases">Changelog</a> - <a href="https://github.com/janhq/cortex/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> ⚠️ **Cortex is currently in Development**: Expect breaking changes and bugs!

## About
Cortex is an openAI-compatible local AI server that developers can use to build LLM apps. It is packaged with a Docker-inspired command-line interface and a Typescript client library. It can be used as a standalone server, or imported as a library. 

Cortex currently supports two inference engines:

- Llama.cpp
- TensorRT-LLM

> Read more about Cortex at https://jan.ai/cortex

## Quicklinks
**Cortex**:
- [Website](https://jan.ai/)
- [GitHub](https://github.com/janhq/cortex)
- [User Guides](https://jan.ai/cortex)
- [API reference](https://jan.ai/api-reference)

## Prerequisites

### **Dependencies**

Before installation, ensure that you have installed the following:

- **Node.js**: Required for running the installation.
- **NPM**: Needed to manage packages.
- **CPU Instruction Sets**: Available for download from the [Cortex GitHub Releases](https://github.com/janhq/cortex/releases) page.


>💡 The **CPU instruction sets** are not required for the initial installation of Cortex. This dependency will be automatically installed during the Cortex initialization if they are not already on your system.


### **Hardware**

Ensure that your system meets the following requirements to run Cortex:

- **OS**:
  - MacOSX 13.6 or higher.
  - Windows 10 or higher.
  - Ubuntu 12.04 and later.
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
1. Install the Cortex NPM package:
``` bash
npm i -g @janhq/cortex
```

2. Initialize a compatible engine:
``` bash
cortex init
```

3. Download a GGUF model from Hugging Face:
``` bash
cortex models pull janhq/TinyLlama-1.1B-Chat-v1.0-GGUF
```
4. Load the model:
``` bash
cortex models start janhq/TinyLlama-1.1B-Chat-v1.0-GGUF
```

5. Start chatting with the model:
``` bash
cortex chat tell me a joke
```


## Run as an API server
To run Cortex as an API server:
```bash
cortex serve
```

## Build from Source

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

## Cortex CLI Command
The following CLI commands are currently available:
> ⚠️ **Cortex is currently in Development**: More commands will be added soon!

```bash

  serve               Providing API endpoint for Cortex backend
  chat                Send a chat request to a model
  init|setup          Init settings and download cortex's dependencies
  ps                  Show running models and their status
  kill                Kill running cortex processes
  pull|download       Download a model. Working with HuggingFace model id.
  run [options]       EXPERIMENTAL: Shortcut to start a model and chat
  models              Subcommands for managing models
  models list         List all available models.
  models pull         Download a specified model.
  models remove       Delete a specified model.
  models get          Retrieve the configuration of a specified model.
  models start        Start a specified model.
  models stop         Stop a specified model.
  models update       Update the configuration of a specified model.
```
## Uninstall Cortex

Run the following command to uninstall Cortex globally on your machine:

```
# Uninstall globally using NPM
npm uninstall -g @janhq/cortex
```
## Contact Support
- For support, please file a GitHub ticket.
- For questions, join our Discord [here](https://discord.gg/FTk2MvZwJH).
- For long-form inquiries, please email [hello@jan.ai](mailto:hello@jan.ai).