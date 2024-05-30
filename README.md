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

### Repo Structure
```
# Entity Definitions
domain/                    # This is the core directory where the domains are defined.
  abstracts/               # Abstract base classes for common attributes and methods.
  models/                  # Domain interface definitions, e.g. model, assistant.
  repositories/            # Extensions abstract and interface

# Business Rules
usecases/                  # Application logic 
	assistants/              # CRUD logic (invokes dtos, entities).
	chat/                    # Logic for chat functionalities.
	models/                  # Logic for model operations.

# Adapters & Implementations
infrastructure/            # Implementations for Cortex interactions
  commanders/              # CLI handlers
    models/
    questions/             # CLI installation UX
    shortcuts/             # CLI chained syntax
    types/
    usecases/              # Invokes UseCases

  controllers/             # Nest controllers and HTTP routes
		assistants/						 # Invokes UseCases
	  chat/     						 # Invokes UseCases
		models/                # Invokes UseCases
	
  database/                # Database providers (mysql, sqlite)
	
	# Framework specific object definitions
  dtos/                    # DTO definitions (data transfer & validation)
  entities/                # TypeORM entity definitions (db schema)
  
	# Providers
  providers/cortex         # Cortex [server] provider (a core extension)
  repositories/extensions  # Extension provider (core & external extensions)

extensions/                # External extensions
command.module.ts          # CLI Commands List
main.ts                    # Entrypoint
```
## Installation

### Prerequisites

#### **Dependencies**

Before installation, ensure that you have installed the following:

- **Node.js**: Required for running the installation.
- **NPM**: Needed to manage packages.
- **CPU Instruction Sets**: Available for download from the [Cortex GitHub Releases](https://github.com/janhq/cortex/releases) page.

<aside>
💡 The **CPU instruction sets** are not required for the initial installation of Cortex. This dependency will be automatically installed during the Cortex initialization if they are not already on your system.

</aside>

#### **Hardware**

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

### Cortex Installation

To install Cortex, follow the steps below:

#### Step 1: Install Cortex

Run the following command to install Cortex globally on your machine:

```bash
# Install using NPM globally
npm i -g @janhq/cortex
```

#### Step 2: Verify the Installation

After installation, you can verify that Cortex is installed correctly by getting help information.

```bash
# Get the help information
cortex -h
```

#### Step 3: Initialize Cortex

Once verified, you need to initialize the Cortex engine.

1. Initialize the Cortex engine:

```
cortex init
```

2. Select between `CPU` and `GPU` modes.

```bash
? Select run mode (Use arrow keys)
> CPU
  GPU
```

3. Select between GPU types.

```bash
? Select GPU types (Use arrow keys)
> Nvidia
  Others (Vulkan)
```

4. Select CPU instructions (will be deprecated soon).

```bash
? Select CPU instructions (Use arrow keys)
> AVX2
  AVX
  AVX-512
```

5. Cortex will download the required CPU instruction sets if you choose `CPU` mode. If you choose `GPU` mode, Cortex will download the necessary dependencies to use your GPU.
6. Once downloaded, Cortex is ready to use!

#### Step 4: Pull a model

From HuggingFace

```bash
cortex pull janhq/phi-3-medium-128k-instruct-GGUF
```

From Jan Hub (TBD)

```bash
cortex pull llama3
```

#### Step 5: Chat

```bash
cortex run janhq/phi-3-medium-128k-instruct-GGUF
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
cortex --help
Usage: cortex <command>

Commands:
  chat                Send a query to the chat service.
                      Example: cortex chat "tell me a joke" --stream

  models list         List all available models.
                      Example: cortex models list

  models pull         Download a specified model.
                      Example: cortex models pull llama3:8b

  models remove       Delete a specified model.
                      Example: cortex models remove llama3:8b

  models get          Retrieve the configuration of a specified model.
                      Example: cortex models config llama3:8b

  models start        Start a specified model.
                      Example: cortex models start llama3:8b

  models stop         Stop a specified model.
                      Example: cortex models stop llama3:8b

  models update       Update the configuration of a specified model.
                      Example: cortex models update llama3:8b --ngl 32

  engines             Execute a specified command related to engines.
                      Example: cortex engines llamacpp

  engines list        List all available engines.
                      Example: cortex engines list

Options:
  -h, --help          Show this help message and exit.


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