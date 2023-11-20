---
title: Installation
slug: /install
---

# Nitro Installation Guide

This guide provides instructions for installing Nitro using the provided [install.sh](https://github.com/janhq/nitro/blob/main/install.sh) and [install.bat](https://github.com/janhq/nitro/blob/main/install.bat) scripts for Linux, macOS, and Windows systems.

## Features

The installation script offers the following features:

1. **Root Privilege Check**: Ensures the script is run with root privileges to avoid permission issues.
2. **Dependency Check**: Checks for and advises on the installation of `jq` and `unzip`.
3. **Automated Nitro Installation**: Downloads and installs the appropriate Nitro version based on the user's OS and architecture.
4. **Uninstall Script Creation**: Generates an uninstall script for easy removal of Nitro if needed.
5. **Enhanced User Experience**: Offers clear and colored output messages during the installation process.

## Prerequisites

- **Linux and macOS**: `jq`, `curl` and `sudo` are required. If `sudo` is not available, the user must have passwordless sudo privileges. If `jq` or `curl` are not available, the script will attempt to suggest installation commands for these packages.
- **Windows**: `PowerShell` are required.

- **GPU Version**: GPU is supported on Linux and Windows only. [nvidia-cuda-toolkits-12.x](https://developer.nvidia.com/cuda-toolkit) is required on both Linux and Windows. 

## Installation Instructions

### Linux and macOS

- **Latest version (CPU is default):**

  ```bash
  curl -sfL https://raw.githubusercontent.com/janhq/nitro/main/install.sh | sudo /bin/bash -
  ```

- **Specific Version Installation:**
  ```bash
  curl -sfL https://raw.githubusercontent.com/janhq/nitro/main/install.sh -o /tmp/install.sh && chmod +x /tmp/install.sh && sudo bash /tmp/install.sh --version 0.1.7 && rm /tmp/install.sh
  ```

- **GPU Version Installation:**
  ```bash
  curl -sfL https://raw.githubusercontent.com/janhq/nitro/main/install.sh -o /tmp/install.sh && chmod +x /tmp/install.sh && sudo bash /tmp/install.sh --gpu && rm /tmp/install.sh
  ```

- **GPU Version Installation Specific Version:**
  ```bash
  curl -sfL https://raw.githubusercontent.com/janhq/nitro/main/install.sh -o /tmp/install.sh && chmod +x /tmp/install.sh && sudo bash /tmp/install.sh --gpu --version 0.1.7 && rm /tmp/install.sh
  ```

- **Manual Installation by downloaing the script loacally and run with different arguments:**

    ```bash
    # Download the script
    curl -sfL https://raw.githubusercontent.com/janhq/nitro/main/install.sh -o ./install.sh

    # Make the script executable
    chmod +x ./install.sh

    # Arguments supported
    #   --version: Specify the version to install for example "--version 0.1.7", default is latest, list version of nitro can be found in https://github.com/janhq/nitro/releases
    #   --gpu: Install the GPU version of nitro, default is CPU version

    # Run one of the following commands

    # Download and install the latest version of nitro
    sudo ./install.sh

    # Download and install the specific version of nitro
    sudo ./install.sh --version 0.1.7

    # Download and install the GPU version of nitro
    sudo ./install.sh --gpu

    # Download and install the GPU version of nitro with specific version
    sudo ./install.sh --gpu --version 0.1.7
    ```
### Windows
- **Latest version (CPU is default)**
  ```bash
  powershell -Command "& { Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/janhq/nitro/main/install.bat' -OutFile 'install.bat'; .\install.bat; Remove-Item -Path 'install.bat' }"
  ```

- **Specific Version Installation:**
  ```bash
  powershell -Command "& { Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/janhq/nitro/main/install.bat' -OutFile 'install.bat'; .\install.bat --version 0.1.7; Remove-Item -Path 'install.bat' }"
  ```

- **GPU Version Installation:**
  ```bash
  powershell -Command "& { Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/janhq/nitro/main/install.bat' -OutFile 'install.bat'; .\install.bat --gpu; Remove-Item -Path 'install.bat' }"
  ```

- **GPU Version Installation Specific Version:**
  ```bash
  powershell -Command "& { Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/janhq/nitro/main/install.bat' -OutFile 'install.bat'; .\install.bat --gpu --version 0.1.7; Remove-Item -Path 'install.bat' }"
  ```
- **Manual Installation by downloaing the script loacally and run with different arguments**

    ```bash
    # Download the script
    Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/janhq/nitro/main/install.bat' -OutFile 'install.bat'

    # Arguments supported
    #   --version: Specify the version to install for example "--version 0.1.7", default is latest, list version of nitro can be found in https://github.com/janhq/nitro/releases
    #   --gpu: Install the GPU version of nitro, default is CPU version
    # Run one of the following commands
    # Download and install the latest version of nitro
    .\install.bat

    # Download and install the specific version of nitro
    .\install.bat --version 0.1.7

    # Download and install the GPU version of nitro
    .\install.bat --gpu

    # Download and install the GPU version of nitro with specific version
    .\install.bat --gpu --version 0.1.7
    ```
## Usage
After installation, launch Nitro by typing `nitro` (or `nitro.exe` on Windows) in a new terminal or PowerShell window. This will start the Nitro server.

Simple testcase with nitro, after starting the server, you can run the following command to test the server in a new terminal or powershell session:

- **On Linux and MacOS:**
  ```bash title="Linux and Macos"
  # Download tiny model
  DOWNLOAD_URL=https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v0.3-GGUF/resolve/main/tinyllama-1.1b-chat-v0.3.Q2_K.gguf
  # Check if /tmp/testmodel exists, if not, download it
  if [[ ! -f "/tmp/testmodel" ]]; then
      wget $DOWNLOAD_URL -O /tmp/testmodel
  fi
  # Load the model to nitro
  curl -s --location 'http://localhost:3928/inferences/llamacpp/loadModel' \
  --header 'Content-Type: application/json' \
  --data '{
      "llama_model_path": "/tmp/testmodel",
      "ctx_len": 2048,
      "ngl": 32,
      "embedding": false
  }'
  # Send a prompt request to nitro
  curl -s --location 'http://localhost:3928/inferences/llamacpp/chat_completion' \
  --header 'Content-Type: application/json' \
  --data '{
          "messages": [
              {"content": "Hello there", "role": "assistant"},
              {"content": "Write a long and sad story for me", "role": "user"}
          ],
          "stream": true,
          "max_tokens": 100,
          "stop": ["hello"],
          "frequency_penalty": 0,
          "presence_penalty": 0,
          "temperature": 0.7
      }'
  ```

- **On Windows:**
  ```bash title="Windows"
  # Download tiny model
  set "MODEL_PATH=%TEMP%\testmodel"
  if not exist "%MODEL_PATH%" (
      bitsadmin.exe /transfer "DownloadTestModel" %DOWNLOAD_URL% "%MODEL_PATH%"
  )

  # Load the model to nitro
  call set "MODEL_PATH_STRING=%%MODEL_PATH:\=\\%%"
  set "curl_data1={\"llama_model_path\":\"%MODEL_PATH_STRING%\"}"
  curl.exe -s -w "%%{http_code}" --location "http://localhost:3928/inferences/llamacpp/loadModel" --header "Content-Type: application/json" --data "%curl_data1%"

  # Send a prompt request to nitro
  set "curl_data2={\"messages\":[{\"content\":\"Hello there\",\"role\":\"assistant\"},{\"content\":\"Write a long and sad story for me\",\"role\":\"user\"}],\"stream\":true,\"model\":\"gpt-3.5-turbo\",\"max_tokens\":100,\"stop\":[\"hello\"],\"frequency_penalty\":0,\"presence_penalty\":0,\"temperature\":0.7}"
  curl.exe -s -w "%%{http_code}" --location "http://localhost:3928/inferences/llamacpp/chat_completion" ^
  --header "Content-Type: application/json" ^
  --data "%curl_data2%"
  ```
  
## Uninstallation
- **Linux and macOS**: Run `sudo uninstall_nitro.sh` from anywhere (the script is added to PATH).
- **Windows**: Open PowerShell and run `uninstallnitro.bat` from anywhere (the script is added to PATH).