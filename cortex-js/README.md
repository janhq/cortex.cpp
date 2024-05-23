# Installation

## Prerequisites

### **Dependencies**

Before installation, ensure that you have installed the following:

- **Node.js**: Required for running the installation.
- **NPM**: Needed to manage packages.
- **CPU Instruction Sets**: Available for download from the [Cortex GitHub Releases](https://github.com/janhq/cortex/releases) page.

<aside>
💡 The **CPU instruction sets** are not required for the initial installation of Cortex. This dependency will be automatically installed during the Cortex initialization if they are not already on your system.

</aside>

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

## Cortex Installation

To install Cortex, follow the steps below:

### Step 1: Install Cortex

Run the following command to install Cortex globally on your machine:

```bash
# Install using NPM globally
npm i -g @janhq/cortex
```

### Step 2: Verify the Installation

After installation, you can verify that Cortex is installed correctly by getting help information.

```bash
# Get the help information
cortex -h
```

### Step 3: Initialize Cortex

Once verified, you need to initialize the Cortex engine.

1. Initialize the Cortex engine:

```
cortex init
```

1. Select between `CPU` and `GPU` modes.

```bash
? Select run mode (Use arrow keys)
> CPU
  GPU
```

2. Select between GPU types.

```bash
? Select GPU types (Use arrow keys)
> Nvidia
  Others (Vulkan)
```

3. Select CPU instructions (will be deprecated soon).

```bash
? Select CPU instructions (Use arrow keys)
> AVX2
  AVX
  AVX-512
```

1. Cortex will download the required CPU instruction sets if you choose `CPU` mode. If you choose `GPU` mode, Cortex will download the necessary dependencies to use your GPU.
2. Once downloaded, Cortex is ready to use!

### Step 4: Pull a model

From HuggingFace

```bash
cortex pull janhq/phi-3-medium-128k-instruct-GGUF
```

From Jan Hub (TBD)

```bash
cortex pull llama3
```

### Step 5: Chat

```bash
cortex run janhq/phi-3-medium-128k-instruct-GGUF
```

## Run as an API server

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

1. Make the `command.js` executable:

```bash
chmod +x '[path-to]/cortex/cortex-js/dist/src/command.js'
```

1. Link the package globally:

```bash
npm link
```

## Uninstall Cortex

Run the following command to uninstall Cortex globally on your machine:

```
# Uninstall globally using NPM
npm uninstall -g @janhq/cortex
```
