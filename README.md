# Cortex.cpp

<p align="center">
<img width="1280" alt="Cortex cpp's Readme Banner" src="https://github.com/user-attachments/assets/a27c0435-b3b4-406f-b575-96ac4f12244c">

</p>

<p align="center">
  <!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
  <img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/m/janhq/cortex.cpp"/>
  <img alt="Github Last Commit" src="https://img.shields.io/github/last-commit/janhq/cortex.cpp"/>
  <img alt="Github Contributors" src="https://img.shields.io/github/contributors/janhq/cortex.cpp"/>
  <img alt="GitHub closed issues" src="https://img.shields.io/github/issues-closed/janhq/cortex.cpp"/>
  <img alt="Discord" src="https://img.shields.io/discord/1107178041848909847?label=discord"/>
</p>

<p align="center">
  <a href="https://cortex.so/docs/">Documentation</a> - <a href="https://cortex.so/api-reference">API Reference</a> 
  - <a href="https://github.com/janhq/cortex.cpp/releases">Changelog</a> - <a href="https://github.com/janhq/cortex.cpp/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> ⚠️ **Cortex.cpp is currently in active development. This outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.**

## Overview

Cortex.cpp is a Local AI engine that is used to run and customize LLMs. Cortex can be deployed as a standalone server, or integrated into apps like [Jan.ai](https://jan.ai/).

Cortex.cpp is a multi-engine that uses `llama.cpp` as the default engine but also supports the following:

- [`llamacpp`](https://github.com/janhq/cortex.llamacpp)
- [`onnx`](https://github.com/janhq/cortex.onnx)
- [`tensorrt-llm`](https://github.com/janhq/cortex.tensorrt-llm)

## Installation

This Local Installer packages all required dependencies, so that you don’t need an internet connection during the installation process.

Alternatively, Cortex is available with a [Network Installer](#network-installer) which downloads the necessary dependencies from the internet during the installation.

### Stable

### Windows:

<a href='https://app.cortexcpp.com/download/latest/windows-amd64-local'>
  <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:14px; width: 14px" />
  <b>cortex-local-installer.exe</b>
</a>

### MacOS:

<a href='https://app.cortexcpp.com/download/latest/mac-universal-local'>
  <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
  <b>cortex-local-installer.pkg</b>
</a>

### Linux:

<a href='https://app.cortexcpp.com/download/latest/linux-amd64-local'>
  <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:14px; width: 14px" />
  <b>cortex-local-installer.deb</b>
</a>

Download the installer and run the following command in terminal:

```bash
    sudo apt install ./cortex-local-installer.deb
    # or
    sudo apt install ./cortex-network-installer.deb
```

The binary will be installed in the `/usr/bin/` directory.

## Usage

After installation, you can run Cortex.cpp from the command line by typing `cortex --help`. For Beta preview, you can run `cortex-beta --help`.

## Built-in Model Library

Cortex.cpp supports various models available on the [Cortex Hub](https://huggingface.co/cortexso). Once downloaded, all model source files will be stored in `~\cortexcpp\models`.

Example models:

| Model          | llama.cpp<br >`:gguf` | TensorRT<br >`:tensorrt` | ONNXRuntime<br >`:onnx` | Command                       |
| -------------- | --------------------- | ------------------------ | ----------------------- | ----------------------------- |
| llama3.1       | ✅                    |                          | ✅                      | cortex run llama3.1:gguf      |
| llama3         | ✅                    | ✅                       | ✅                      | cortex run llama3             |
| mistral        | ✅                    | ✅                       | ✅                      | cortex run mistral            |
| qwen2          | ✅                    |                          |                         | cortex run qwen2:7b-gguf      |
| codestral      | ✅                    |                          |                         | cortex run codestral:22b-gguf |
| command-r      | ✅                    |                          |                         | cortex run command-r:35b-gguf |
| gemma          | ✅                    |                          | ✅                      | cortex run gemma              |
| mixtral        | ✅                    |                          |                         | cortex run mixtral:7x8b-gguf  |
| openhermes-2.5 | ✅                    | ✅                       | ✅                      | cortex run openhermes-2.5     |
| phi3 (medium)  | ✅                    |                          | ✅                      | cortex run phi3:medium        |
| phi3 (mini)    | ✅                    |                          | ✅                      | cortex run phi3:mini          |
| tinyllama      | ✅                    |                          |                         | cortex run tinyllama:1b-gguf  |

> **Note**:
> You should have at least 8 GB of RAM available to run the 7B models, 16 GB to run the 14B models, and 32 GB to run the 32B models.

## Cortex.cpp CLI Commands

For complete details on CLI commands, please refer to our [CLI documentation](https://cortex.so/docs/cli).

## REST API

Cortex.cpp includes a REST API accessible at `localhost:39281`. For a complete list of endpoints and their usage, visit our [API documentation](https://cortex.so/api-reference).

## Advanced Installation

### Local Installer: Beta & Nightly Versions

Beta is an early preview for new versions of Cortex. It is for users who want to try new features early - we appreciate your feedback.

Nightly is our development version of Cortex. It is released every night and may contain bugs and experimental features.

<table>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Version Type</b></td>
    <td style="text-align:center"><b>Windows</b></td>
    <td style="text-align:center"><b>MacOS</b></td>
    <td style="text-align:center"><b>Linux</b></td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Stable (Recommended)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/latest/windows-amd64-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:14px; width: 14px" />
        <b>cortex-local-installer.exe</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/latest/mac-universal-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        <b>cortex-local-installer.pkg</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/latest/linux-amd64-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:14px; width: 14px" />
        <b>cortex-local-installer.deb</b>
      </a>
    </td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Beta (Preview)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/windows-amd64-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:14px; width: 14px" />
        <b>cortex-local-installer.exe</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/mac-universal-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        <b>cortex-local-installer.pkg</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/linux-amd64-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:14px; width: 14px" />
        <b>cortex-local-installer.deb</b>
      </a>
    </td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Nightly Build (Experimental)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/windows-amd64-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:14px; width: 14px" />
        <b>cortex-local-installer.exe</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/mac-universal-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        <b>cortex-local-installer.pkg</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/linux-amd64-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:14px; width: 14px" />
        <b>cortex-local-installer.deb</b>
      </a>
    </td>
  </tr>
</table>

### Network Installer

Cortex.cpp is available with a Network Installer, which is a smaller installer but requires internet connection during installation to download the necessary dependencies.

<table>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Version Type</b></td>
    <td style="text-align:center"><b>Windows</b></td>
    <td style="text-align:center"><b>MacOS</b></td>
    <td style="text-align:center"><b>Linux</b></td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Stable (Recommended)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/latest/windows-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:14px; width: 14px" />
        <b>cortex-network-installer.exe</b>
      </a>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/latest/mac-universal-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        <b>cortex-network-installer.pkg</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/latest/linux-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:14px; width: 14px" />
       <b>cortex-network-installer.deb</b>
      </a>
    </td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Beta (Preview)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/windows-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:14px; width: 14px" />
        <b>cortex-network-installer.exe</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/mac-universal-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        <b>cortex-network-installer.pkg</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/linux-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:14px; width: 14px" />
        <b>cortex-network-installer.deb</b>
      </a>
    </td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Nightly Build (Experimental)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/windows-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:14px; width: 14px" />
        <b>cortex-network-installer.exe</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/mac-universal-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        <b>cortex-network-installer.pkg</b>
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/linux-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:14px; width: 14px" />
        <b>cortex-network-installer.deb</b>
      </a>
    </td>
  </tr>
</table>

### Build from Source

#### Windows

1. Clone the Cortex.cpp repository [here](https://github.com/janhq/cortex.cpp).
2. Navigate to the `engine` folder.
3. Configure the vpkg:

```bash
cd vcpkg
./bootstrap-vcpkg.bat
vcpkg install
```

4. Build the Cortex.cpp inside the `engine/build` folder:

```bash
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=path_to_vcpkg_folder_in_cortex_repo/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build . --config Release
```

```sh
# Get the help information
cortex -h
```

#### MacOS

1. Clone the Cortex.cpp repository [here](https://github.com/janhq/cortex.cpp).
2. Navigate to the `engine` folder.
3. Configure the vpkg:

```bash
cd vcpkg
./bootstrap-vcpkg.sh
vcpkg install
```

4. Build the Cortex.cpp inside the `engine/build` folder:

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=path_to_vcpkg_folder_in_cortex_repo/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j4
```

```sh
# Get the help information
cortex -h
```

#### Linux

1. Clone the Cortex.cpp repository [here](https://github.com/janhq/cortex.cpp).
2. Navigate to the `engine` folder.
3. Configure the vpkg:

```bash
cd vcpkg
./bootstrap-vcpkg.sh
vcpkg install
```

4. Build the Cortex.cpp inside the `engine/build` folder:

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=path_to_vcpkg_folder_in_cortex_repo/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j4
```

```sh
# Get help
cortex
```

## Uninstallation

### Windows

1. Open the Windows Control Panel.
2. Navigate to `Add or Remove Programs`.
3. Search for `cortexcpp` and double click to uninstall. (for beta and nightly builds, search for `cortexcpp-beta` and `cortexcpp-nightly` respectively)

### MacOs

Run the uninstaller script:

```bash
sudo sh cortex-uninstall.sh
```

For MacOS, there is a uninstaller script comes with the binary and added to the `/usr/local/bin/` directory. The script is named `cortex-uninstall.sh` for stable builds, `cortex-beta-uninstall.sh` for beta builds and `cortex-nightly-uninstall.sh` for nightly builds.

### Linux

```bash
# For stable builds
sudo apt remove cortexcpp
```

## Contact Support

- For support, please file a [GitHub ticket](https://github.com/janhq/cortex.cpp/issues/new/choose).
- For questions, join our Discord [here](https://discord.gg/FTk2MvZwJH).
- For long-form inquiries, please email [hello@jan.ai](mailto:hello@jan.ai).
