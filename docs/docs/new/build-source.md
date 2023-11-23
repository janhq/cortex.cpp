---
title: Build From Source
slug: /build-source
---

This guide provides step-by-step instructions for building Nitro from source on Linux, macOS, and Windows systems.

## Clone the Repository

First, you need to clone the Nitro repository:

```bash
git clone --recurse https://github.com/janhq/nitro
```

If you don't have git, you can download the source code as a file archive from [Nitro GitHub](https://github.com/janhq/nitro). Each [release](https://github.com/caddyserver/caddy/releases) also has source snapshots.

## Install Dependencies

Next, let's install the necessary dependencies.

- **On MacOS with Apple Silicon:**

  ```bash
  ./install_deps.sh
  ```

- **On Windows:**

  ```bash
  cmake -S ./nitro_deps -B ./build_deps/nitro_deps
  cmake --build ./build_deps/nitro_deps --config Release
  ```

This creates a `build_deps` folder.

## Generate build file

Now, let's generate the build files.

- **On MacOS, Linux, and Windows:**

  ```bash
  mkdir build && cd build
  cmake ..
  ```

- **On MacOS with Intel processors:**

  ```bash
  mkdir build && cd build
  cmake -DLLAMA_METAL=OFF ..
  ```

- **On Linux with CUDA:**

  ```bash
  mkdir build && cd build
  cmake -DLLAMA_CUBLAS=ON ..
  ```

## Build the Application

Time to build Nitro!

- **On MacOS:**

  ```bash
  make -j $(sysctl -n hw.physicalcpu)
  ```

- **On Linux:**

  ```bash
  make -j $(nproc)
  ```

- **On Windows:**

  ```bash
  make -j $(%NUMBER_OF_PROCESSORS%)
  ```

## Start process

Finally, let's start Nitro.

- **On MacOS and Linux:**

  ```bash
  ./nitro
  ```

- **On Windows:**

  ```bash
  cd Release
  copy ..\..\build_deps\_install\bin\zlib.dll .
  nitro.exe
  ```

To verify if the build was successful:

```bash
curl http://localhost:3928/healthz
```
